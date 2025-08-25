# cpu-governor

> 🚀 **Minimal, fast CPU frequency governor controller written in C**

A lightweight, zero-dependency tool for controlling CPU governors on Linux systems. Designed for minimal system interruption with intelligent core management.

## 🎯 Features

- **Minimal**: Single C file, ~8KB compiled binary
- **Fast**: Direct sysfs access, no shell overhead  
- **Safe**: Input validation and error handling
- **Universal**: Works on all Linux systems with CPU frequency scaling
- **Intelligent**: Auto-detects available governors and CPU cores
- **Zero Dependencies**: Pure C with standard library only

## 📦 Quick Start

### Download & Compile
```bash
# Clone or download the cpu-governor.c file
git clone https://github.com/0xb0rn3/cpu-governor/
cd cpu-governor

# Compile (requires gcc)
gcc -O2 -o cpu-governor cpu-governor.c

# Make executable
chmod +x cpu-governor

# First time setup - install system-wide (optional)
sudo ./cpu-governor install
```

### Immediate Usage
```bash
# Check current status (no root required)
./cpu-governor status

# Set maximum performance (requires sudo)
sudo ./cpu-governor performance

# Set power saving mode
sudo ./cpu-governor powersave

# Set other governors
sudo ./cpu-governor ondemand
sudo ./cpu-governor conservative
sudo ./cpu-governor schedutil
```

## 🔧 Compilation Guide

### Prerequisites
- GCC compiler
- Linux system with CPU frequency scaling support
- Standard C library (glibc)

### Build Commands

#### Basic Build
```bash
gcc -o cpu-governor cpu-governor.c
```

#### Optimized Build (Recommended)
```bash
gcc -O2 -march=native -o cpu-governor cpu-governor.c
```

#### Static Build (Portable)
```bash
gcc -O2 -static -o cpu-governor cpu-governor.c
```

#### Debug Build
```bash
gcc -g -DDEBUG -o cpu-governor cpu-governor.c
```

### Cross-Compilation Examples
```bash
# For ARM64
aarch64-linux-gnu-gcc -O2 -o cpu-governor-arm64 cpu-governor.c

# For ARM32
arm-linux-gnueabihf-gcc -O2 -o cpu-governor-arm32 cpu-governor.c
```

## 📚 Usage Guide

### Command Syntax
```
cpu-governor <command|governor>
```

### Available Commands

| Command | Description | Root Required |
|---------|-------------|---------------|
| `status` | Show current governor and CPU info | ❌ |
| `performance` | Set performance mode + enable boost | ✅ |
| `powersave` | Set powersave mode + disable boost | ✅ |
| `ondemand` | Set ondemand governor | ✅ |
| `conservative` | Set conservative governor | ✅ |
| `schedutil` | Set schedutil governor | ✅ |
| `userspace` | Set userspace governor | ✅ |
| `install` | Install system-wide to `/usr/local/bin` | ✅ |
| `help` | Show usage information | ❌ |

### Governor Descriptions

- **performance**: Maximum CPU frequency at all times - best for gaming, benchmarks, compilation
- **powersave**: Minimum CPU frequency - best for battery life, low-load scenarios  
- **ondemand**: Dynamic scaling based on CPU load - good general-purpose balance
- **conservative**: Gradual frequency changes - smoother than ondemand
- **schedutil**: Kernel scheduler-guided scaling - modern default on recent kernels
- **userspace**: Manual frequency control - for advanced users

### Example Workflows

#### Gaming/High Performance
```bash
# Before gaming session
sudo cpu-governor performance

# Check it's applied
cpu-governor status

# After gaming (optional)
sudo cpu-governor ondemand
```

#### Battery Optimization
```bash
# Maximize battery life
sudo cpu-governor powersave

# Verify power savings
cpu-governor status
```

#### Development/Compilation
```bash
# Fast compilation
sudo cpu-governor performance

# Normal operation
sudo cpu-governor schedutil
```

## 🛠️ Installation Options

### Option 1: System-wide Installation (Recommended)
```bash
# Compile first
gcc -O2 -o cpu-governor cpu-governor.c

# Install to /usr/local/bin (requires sudo)
sudo ./cpu-governor install

# Now available globally
cpu-governor status
sudo cpu-governor performance
```

### Option 2: Local Installation
```bash
# Place in your PATH
mkdir -p ~/bin
cp cpu-governor ~/bin/
echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### Option 3: Direct Usage
```bash
# Use directly from compilation directory
./cpu-governor status
sudo ./cpu-governor performance
```

## 💡 Advanced Usage

### XFCE Integration
Create desktop shortcuts or panel launchers:

```bash
# Performance mode launcher
[Desktop Entry]
Name=CPU Performance
Exec=pkexec cpu-governor performance
Icon=cpu
Type=Application

# Power saving launcher  
[Desktop Entry]
Name=CPU PowerSave
Exec=pkexec cpu-governor powersave
Icon=battery
Type=Application
```

### Scripting Integration
```bash
#!/bin/bash
# Gaming script
echo "Enabling performance mode..."
sudo cpu-governor performance

# Launch game
steam steam://rungameid/12345

# Restore balanced mode
sudo cpu-governor ondemand
```

### Systemd Service (Auto-apply on boot)
```ini
# /etc/systemd/system/cpu-performance.service
[Unit]
Description=Set CPU Performance Mode
After=multi-user.target

[Service]
Type=oneshot
ExecStart=/usr/local/bin/cpu-governor performance
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
```

Enable with:
```bash
sudo systemctl enable cpu-performance.service
```

## 🔍 Troubleshooting

### Common Issues

**Error: "Cannot read available governors"**
```bash
# Check if CPU frequency scaling is available
ls /sys/devices/system/cpu/cpu0/cpufreq/

# If missing, load modules
sudo modprobe cpufreq_conservative
sudo modprobe cpufreq_ondemand
```

**Error: "Setting governors requires root privileges"**
```bash
# Always use sudo for governor changes
sudo cpu-governor performance

# Check current status without sudo
cpu-governor status
```

**Governor not available**
```bash
# Check what's available on your system
cpu-governor status

# Some governors may not be compiled in your kernel
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
```

### Verification
```bash
# Verify governor is applied
cpu-governor status

# Check if boost is working (Intel)
cat /sys/devices/system/cpu/intel_pstate/no_turbo

# Check if boost is working (AMD/Generic)
cat /sys/devices/system/cpu/cpufreq/boost
```

## 🏗️ Building from Source

### Repository Structure
```
cpu-governor/
├── cpu-governor.c          # Main source file
├── README.md              # This file
├── LICENSE                # MIT License
├── Makefile              # Build automation
└── examples/             # Usage examples
    ├── gaming-script.sh
    ├── battery-save.sh
    └── xfce-launchers/
```

### Makefile (Optional)
```makefile
CC=gcc
CFLAGS=-O2 -Wall -Wextra
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

cpu-governor: cpu-governor.c
	$(CC) $(CFLAGS) -o $@ $<

install: cpu-governor
	install -D cpu-governor $(BINDIR)/cpu-governor

clean:
	rm -f cpu-governor

.PHONY: install clean
```

Build with:
```bash
make
sudo make install
```

## 📊 Performance Impact

- **Binary size**: ~8KB compiled
- **Memory usage**: <1MB RSS
- **Execution time**: <10ms typical
- **System calls**: Minimal (direct sysfs access)
- **Dependencies**: None (static linking possible)

## 🔒 Security Considerations

- Requires root privileges for governor changes (by design)
- Uses standard sysfs interfaces (no kernel modules)
- Input validation prevents invalid governors
- No network access or external dependencies
- Can be run with `pkexec` for GUI integration

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on multiple distributions
5. Submit a pull request

## 📄 License

MIT License - see LICENSE file for details

## 👨‍💻 Author

**0xb0rn3** | **0xbv1**
- Discord: `oxbv1`
- X/Twitter: `@oxbv1`
- GitHub: `@0xb0rn3`

---

**Made with ⚡ for Linux power users who demand speed and simplicity**
