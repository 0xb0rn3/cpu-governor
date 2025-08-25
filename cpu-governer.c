#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_PATH 256
#define MAX_BUFFER 1024
#define INSTALL_PATH "/usr/local/bin/cpu-governor"

// ANSI color codes
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"

typedef struct {
    char name[32];
    char description[64];
} governor_info;

static const governor_info governors[] = {
    {"performance", "Maximum performance, highest frequencies"},
    {"powersave", "Power saving, lowest frequencies"},
    {"ondemand", "Dynamic scaling based on CPU load"},
    {"conservative", "Conservative frequency scaling"},
    {"schedutil", "Scheduler-guided frequency scaling"},
    {"userspace", "User-controlled frequency scaling"}
};

static const int num_governors = sizeof(governors) / sizeof(governor_info);

void print_colored(const char* color, const char* prefix, const char* message) {
    printf("%s[%s]%s %s\n", color, prefix, RESET, message);
}

int is_root() {
    return getuid() == 0;
}

int file_exists(const char* path) {
    return access(path, F_OK) == 0;
}

int read_file(const char* path, char* buffer, size_t size) {
    FILE* file = fopen(path, "r");
    if (!file) return 0;
    
    if (fgets(buffer, size, file)) {
        // Remove newline
        char* newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
}

int write_file(const char* path, const char* value) {
    FILE* file = fopen(path, "w");
    if (!file) return 0;
    
    int result = fprintf(file, "%s", value) > 0;
    fclose(file);
    return result;
}

int get_cpu_count() {
    glob_t glob_result;
    int count = 0;
    
    if (glob("/sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_governor", 
             GLOB_NOSORT, NULL, &glob_result) == 0) {
        count = glob_result.gl_pathc;
        globfree(&glob_result);
    }
    return count;
}

void show_current_status() {
    char buffer[MAX_BUFFER];
    char available[MAX_BUFFER];
    
    printf("%s=== CPU Governor Status ===%s\n", BLUE, RESET);
    
    // Current governor
    if (read_file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", buffer, sizeof(buffer))) {
        printf("Current Governor: %s%s%s\n", GREEN, buffer, RESET);
    } else {
        print_colored(RED, "ERROR", "Cannot read current governor");
        return;
    }
    
    // Available governors
    if (read_file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors", available, sizeof(available))) {
        printf("Available: %s\n", available);
    }
    
    // CPU count
    int cpu_count = get_cpu_count();
    printf("CPU Cores: %d\n", cpu_count);
    
    // Current frequencies (first 4 cores to avoid spam)
    printf("Frequencies (MHz): ");
    for (int i = 0; i < 4 && i < cpu_count; i++) {
        char freq_path[MAX_PATH];
        snprintf(freq_path, sizeof(freq_path), 
                "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", i);
        
        if (read_file(freq_path, buffer, sizeof(buffer))) {
            int freq_mhz = atoi(buffer) / 1000;
            printf("CPU%d:%d ", i, freq_mhz);
        }
    }
    if (cpu_count > 4) printf("...");
    printf("\n");
}

int validate_governor(const char* governor) {
    char available[MAX_BUFFER];
    
    if (!read_file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors", 
                   available, sizeof(available))) {
        print_colored(RED, "ERROR", "Cannot read available governors");
        return 0;
    }
    
    return strstr(available, governor) != NULL;
}

int set_governor(const char* governor) {
    glob_t glob_result;
    int success_count = 0;
    int total_count = 0;
    
    if (!validate_governor(governor)) {
        printf("%s[ERROR]%s Governor '%s' not available\n", RED, RESET, governor);
        return 0;
    }
    
    printf("%s[INFO]%s Setting governor to: %s\n", BLUE, RESET, governor);
    
    if (glob("/sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_governor", 
             GLOB_NOSORT, NULL, &glob_result) == 0) {
        
        for (size_t i = 0; i < glob_result.gl_pathc; i++) {
            if (write_file(glob_result.gl_pathv[i], governor)) {
                success_count++;
            }
            total_count++;
        }
        globfree(&glob_result);
    }
    
    if (success_count == total_count && total_count > 0) {
        printf("%s[SUCCESS]%s Applied to %d CPU cores\n", GREEN, RESET, total_count);
        return 1;
    } else {
        printf("%s[ERROR]%s Failed on some cores (%d/%d)\n", 
               RED, RESET, success_count, total_count);
        return 0;
    }
}

void set_cpu_boost(int enable) {
    const char* boost_files[] = {
        "/sys/devices/system/cpu/cpufreq/boost",
        "/sys/devices/system/cpu/intel_pstate/no_turbo"
    };
    
    for (int i = 0; i < 2; i++) {
        if (file_exists(boost_files[i])) {
            const char* value = (i == 0) ? (enable ? "1" : "0") : (enable ? "0" : "1");
            if (write_file(boost_files[i], value)) {
                printf("%s[INFO]%s CPU boost %s\n", 
                       BLUE, RESET, enable ? "enabled" : "disabled");
                return;
            }
        }
    }
}

void performance_mode() {
    set_governor("performance");
    set_cpu_boost(1);
}

void powersave_mode() {
    set_governor("powersave");
    set_cpu_boost(0);
}

int install_systemwide() {
    char exe_path[MAX_PATH];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    
    if (len == -1) {
        print_colored(RED, "ERROR", "Cannot determine executable path");
        return 0;
    }
    exe_path[len] = '\0';
    
    char command[MAX_PATH * 2];
    snprintf(command, sizeof(command), "cp \"%s\" \"%s\"", exe_path, INSTALL_PATH);
    
    if (system(command) == 0 && chmod(INSTALL_PATH, 0755) == 0) {
        printf("%s[SUCCESS]%s Installed to %s\n", GREEN, RESET, INSTALL_PATH);
        printf("You can now use: cpu-governor performance\n");
        return 1;
    } else {
        print_colored(RED, "ERROR", "Installation failed");
        return 0;
    }
}

void show_usage() {
    printf("%sCPU Governor - Minimal CPU frequency control%s\n\n", BLUE, RESET);
    
    printf("%sUsage:%s\n", YELLOW, RESET);
    printf("  %s <governor>     - Set CPU governor\n", "cpu-governor");
    printf("  %s status         - Show current status\n", "cpu-governor");
    printf("  %s install        - Install system-wide (requires sudo)\n", "cpu-governor");
    printf("  %s help           - Show this help\n\n", "cpu-governor");
    
    printf("%sGovernors:%s\n", YELLOW, RESET);
    for (int i = 0; i < num_governors; i++) {
        printf("  %-12s - %s\n", governors[i].name, governors[i].description);
    }
    
    printf("\n%sExamples:%s\n", YELLOW, RESET);
    printf("  sudo cpu-governor performance  # Max performance\n");
    printf("  sudo cpu-governor powersave    # Power saving\n");
    printf("  cpu-governor status            # Check status\n");
    
    printf("\n%sFirst time setup:%s\n", YELLOW, RESET);
    printf("  sudo ./cpu-governor install\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_usage();
        return 1;
    }
    
    const char* command = argv[1];
    
    if (strcmp(command, "help") == 0 || strcmp(command, "-h") == 0) {
        show_usage();
        return 0;
    }
    
    if (strcmp(command, "status") == 0) {
        show_current_status();
        return 0;
    }
    
    if (strcmp(command, "install") == 0) {
        if (!is_root()) {
            print_colored(RED, "ERROR", "Installation requires root privileges (use sudo)");
            return 1;
        }
        return install_systemwide() ? 0 : 1;
    }
    
    // Governor commands require root
    if (!is_root()) {
        print_colored(RED, "ERROR", "Setting governors requires root privileges (use sudo)");
        printf("Use 'cpu-governor status' to check current settings\n");
        return 1;
    }
    
    // Handle governor setting
    if (strcmp(command, "performance") == 0) {
        performance_mode();
    } else if (strcmp(command, "powersave") == 0) {
        powersave_mode();
    } else {
        // Try setting as generic governor
        if (!set_governor(command)) {
            printf("Use 'cpu-governor help' for usage information\n");
            return 1;
        }
    }
    
    return 0;
}
