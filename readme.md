# WaleedShell

A powerful, feature-rich command-line shell for Windows built from scratch in modern C++20. WaleedShell provides an enhanced terminal experience with built-in system administration tools, process management, network utilities, and more.

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![C++](https://img.shields.io/badge/C%2B%2B-20-orange.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## Features

### Core Shell Features

- **Command History** - Navigate previous commands with UP/DOWN arrow keys
- **Tab Autocomplete** - Auto-complete file paths and executables
- **Alias System** - Create custom command shortcuts
- **Pipe Support** - Chain commands together (`cmd1 | cmd2 | cmd3`)
- **I/O Redirection** - Redirect output to files (`>`, `>>`, `<`)
- **Environment Variables** - View and modify environment variables

### Built-in Modules

| Module               | Description                               |
| -------------------- | ----------------------------------------- |
| **Process Manager**  | List, kill, and start processes           |
| **File Manager**     | File operations without external commands |
| **System Info**      | CPU, memory, disk information             |
| **Registry Manager** | Query and modify Windows Registry         |
| **Network Manager**  | Adapters, connections, ping, DNS lookup   |
| **Service Manager**  | Control Windows services                  |

## Installation

### Prerequisites

- **Windows 10/11**
- **MinGW-w64** (GCC compiler for Windows)

### Installing MinGW-w64

1. Install MSYS2:

```powershell
winget install -e --id MSYS2.MSYS2
```

2. Open **MSYS2 UCRT64** as Administrator and run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
```

3. Add to PATH:
   - Open System Properties (`Win + R` → `sysdm.cpl`)
   - Go to **Advanced** → **Environment Variables**
   - Add `C:\msys64\ucrt64\bin` to your User PATH

4. Verify installation:

```bash
g++ --version
```

### Building WaleedShell

1. Clone the repository:

```bash
git clone https://github.com/yourusername/WaleedShell.git
cd WaleedShell
```

2. Build:

```bash
build.bat
```

3. Run:

```bash
bin\wshell.exe
```

## Usage

### General Commands

| Command           | Description                | Example              |
| ----------------- | -------------------------- | -------------------- |
| `help`            | Show all commands          | `help`               |
| `exit`            | Exit the shell             | `exit`               |
| `clear` / `cls`   | Clear screen               | `clear`              |
| `cd <dir>`        | Change directory           | `cd C:\Users`        |
| `pwd`             | Print working directory    | `pwd`                |
| `history`         | Show command history       | `history`            |
| `alias <n>=<cmd>` | Create alias               | `alias ll=dir /w`    |
| `unalias <name>`  | Remove alias               | `unalias ll`         |
| `which <cmd>`     | Find executable path       | `which notepad`      |
| `env`             | Show environment variables | `env`                |
| `export <N>=<V>`  | Set environment variable   | `export PATH=C:\bin` |

### Process Management

| Command            | Description        | Example                       |
| ------------------ | ------------------ | ----------------------------- |
| `ps`               | List all processes | `ps`                          |
| `kill <pid\|name>` | Terminate process  | `kill 1234` or `kill notepad` |
| `start <cmd>`      | Start new process  | `start notepad`               |
| `pinfo <pid>`      | Process details    | `pinfo 1234`                  |

### File Operations

| Command            | Description           | Example                  |
| ------------------ | --------------------- | ------------------------ |
| `ls [path]`        | List directory        | `ls` or `ls C:\Users`    |
| `cat <file>`       | Display file contents | `cat readme.txt`         |
| `touch <file>`     | Create empty file     | `touch newfile.txt`      |
| `rm <file>`        | Delete file           | `rm oldfile.txt`         |
| `cp <src> <dst>`   | Copy file             | `cp file1.txt file2.txt` |
| `mv <src> <dst>`   | Move/rename file      | `mv old.txt new.txt`     |
| `mkdir <dir>`      | Create directory      | `mkdir newfolder`        |
| `rmdir <dir> [-r]` | Delete directory      | `rmdir folder -r`        |
| `find <pattern>`   | Find files            | `find *.txt`             |
| `finfo <file>`     | File information      | `finfo document.pdf`     |

### System Information

| Command    | Description        | Example    |
| ---------- | ------------------ | ---------- |
| `sysinfo`  | System information | `sysinfo`  |
| `meminfo`  | Memory usage       | `meminfo`  |
| `diskinfo` | Disk space         | `diskinfo` |
| `uptime`   | System uptime      | `uptime`   |

### Registry Operations

| Command               | Description        | Example                        |
| --------------------- | ------------------ | ------------------------------ |
| `reg query <key>`     | Query registry key | `reg query HKCU\Software`      |
| `reg add <k> <v> <d>` | Set registry value | `reg add HKCU\Test Name Value` |
| `reg delete <k> [v]`  | Delete key/value   | `reg delete HKCU\Test Name`    |

### Network Utilities

| Command          | Description           | Example              |
| ---------------- | --------------------- | -------------------- |
| `adapters`       | List network adapters | `adapters`           |
| `netstat`        | Active connections    | `netstat`            |
| `ping <host>`    | Ping host             | `ping google.com`    |
| `resolve <host>` | DNS lookup            | `resolve github.com` |

### Service Management

| Command              | Description       | Example               |
| -------------------- | ----------------- | --------------------- |
| `services`           | List all services | `services`            |
| `services -r`        | List running only | `services -r`         |
| `svc start <name>`   | Start service     | `svc start Spooler`   |
| `svc stop <name>`    | Stop service      | `svc stop Spooler`    |
| `svc restart <name>` | Restart service   | `svc restart Spooler` |
| `svc info <name>`    | Service details   | `svc info Spooler`    |

### Piping and Redirection

```bash
# Pipe output between commands
ps | findstr chrome
dir | findstr .txt
env | findstr PATH

# Redirect output to file
dir > files.txt
echo Hello >> log.txt

# Input redirection
sort < unsorted.txt
```

### Keyboard Shortcuts

| Key         | Action                         |
| ----------- | ------------------------------ |
| `↑` / `↓`   | Navigate command history       |
| `←` / `→`   | Move cursor                    |
| `Tab`       | Autocomplete                   |
| `Home`      | Jump to line start             |
| `End`       | Jump to line end               |
| `Backspace` | Delete character before cursor |
| `Delete`    | Delete character at cursor     |

## Project Structure

```
WaleedShell/
├── bin/
│   └── wshell.exe          # Compiled executable
├── include/
│   └── common.hpp          # Common headers and definitions
├── src/
│   ├── main.cpp            # Entry point
│   ├── shell.hpp           # Shell class declaration
│   ├── shell.cpp           # Shell implementation
│   ├── parser.hpp          # Command parser declaration
│   ├── parser.cpp          # Tokenizer and parser
│   ├── executor.hpp        # Command executor declaration
│   ├── executor.cpp        # Process creation and piping
│   ├── input.hpp           # Input handler declaration
│   ├── input.cpp           # History and autocomplete
│   └── modules/
│       ├── process.hpp     # Process manager
│       ├── process.cpp
│       ├── files.hpp       # File manager
│       ├── files.cpp
│       ├── sysinfo.hpp     # System information
│       ├── sysinfo.cpp
│       ├── registry.hpp    # Registry manager
│       ├── registry.cpp
│       ├── network.hpp     # Network utilities
│       ├── network.cpp
│       ├── services.hpp    # Service manager
│       └── services.cpp
├── build.bat               # Build script
└── README.md
```

## Technical Details

### Windows APIs Used

| Module   | APIs                                                                                            |
| -------- | ----------------------------------------------------------------------------------------------- |
| Process  | `CreateProcess`, `OpenProcess`, `TerminateProcess`, `EnumProcesses`, `CreateToolhelp32Snapshot` |
| Files    | `FindFirstFile`, `CreateFile`, `ReadFile`, `WriteFile`, `CopyFile`, `DeleteFile`                |
| System   | `GetSystemInfo`, `GlobalMemoryStatusEx`, `GetDiskFreeSpaceEx`                                   |
| Registry | `RegOpenKeyEx`, `RegQueryValueEx`, `RegSetValueEx`, `RegEnumKeyEx`                              |
| Network  | `GetAdaptersInfo`, `GetExtendedTcpTable`, `IcmpSendEcho`, `gethostbyname`                       |
| Services | `OpenSCManager`, `EnumServicesStatus`, `StartService`, `ControlService`                         |
| Console  | `ReadConsoleInput`, `SetConsoleMode`, `GetConsoleScreenBufferInfo`                              |

### Libraries Linked

- `psapi` - Process Status API
- `iphlpapi` - IP Helper API
- `ws2_32` - Winsock2

### Compiler Flags

```
-std=c++20    # C++20 standard
-Wall         # All warnings
-Wextra       # Extra warnings
-static       # Static linking
```

## Why This Project Matters

### Technical Significance

#### 1. **Deep Windows Internals Knowledge**
This project directly interacts with the Windows API at the lowest level:
- `CreateProcess` — how Windows actually spawns programs
- `ReadConsoleInput` — raw keyboard input handling
- Handle management (HANDLE, pipes, inheritance)
- Security descriptors and process tokens

This is how tools like PowerShell, CMD, and Windows Terminal work internally.

#### 2. **Systems Programming Skills**
Core OS concepts built from scratch:

| Concept | What Was Built |
|---------|----------------|
| Process creation | `CreateProcess` with pipes and redirection |
| IPC (Inter-Process Communication) | Anonymous pipes for `cmd1 \| cmd2` |
| Memory management | Handle lifecycle, buffer management |
| Console I/O | Raw input mode, cursor control |

#### 3. **No Dependencies**
This is pure C++ with Windows SDK — no frameworks, no libraries like Boost. Everything is handcrafted, giving full control and understanding.

---

### Practical Significance

#### For Your Career
- **Systems/Infrastructure roles** — Demonstrates low-level OS knowledge
- **Security research** — Understanding process injection, handle manipulation
- **DevOps/SRE** — Building custom tooling
- **Game development** — Process management, memory control

#### For Your Portfolio
This stands out because:
- It's not a tutorial copy-paste project
- Shows C++ proficiency beyond basic OOP
- Demonstrates Windows API expertise (rare skill)
- Complete, functional, documented

---

### What This Project Actually Is

```
┌─────────────────────────────────────────────────────┐
│                   WaleedShell                       │
├─────────────────────────────────────────────────────┤
│  A custom terminal that can:                        │
│  • Run any Windows program                          │
│  • Pipe data between processes                      │
│  • Manage system processes (like Task Manager)      │
│  • Query/modify Windows Registry (like regedit)     │
│  • Monitor network connections (like netstat)       │
│  • Control Windows services (like services.msc)     │
│  • All from a single executable (~200KB)            │
└─────────────────────────────────────────────────────┘
```

---

### Skills Demonstrated

| Skill | Evidence |
|-------|----------|
| Modern C++ (C++20) | Smart pointers, lambdas, structured bindings |
| Windows API | 50+ API calls across 6 modules |
| Process management | CreateProcess, pipes, handle inheritance |
| Text parsing | Tokenizer with quote handling, pipes, redirects |
| Data structures | Command history, alias maps, process lists |
| CLI/UX design | Tab completion, arrow key navigation |
| Code organization | Modular architecture, separation of concerns |

---

### Real-World Comparison

WaleedShell has features found in professional tools:

| Feature | WaleedShell | CMD | PowerShell |
|---------|-------------|-----|------------|
| Tab completion | ✓ | ✓ | ✓ |
| Command history | ✓ | ✓ | ✓ |
| Piping | ✓ | ✓ | ✓ |
| Aliases | ✓ | ✗ | ✓ |
| Built-in process manager | ✓ | ✗ | ✓ |
| Built-in network tools | ✓ | ✗ | ✓ |
| Single portable EXE | ✓ | ✓ | ✗ |

---

### Interview Value

If asked "tell me about a challenging project":

> "I built a Windows command shell from scratch in C++. It handles process creation with CreateProcess, implements piping using anonymous pipes, has a custom tokenizer for parsing commands with quotes and redirects, and includes modules for process management, registry access, and network monitoring — all using raw Windows APIs with no external libraries."

This demonstrates understanding of how operating systems work at a fundamental level, not just how to use high-level frameworks.

## Building from Source

### Requirements

- C++20 compatible compiler (GCC 10+ or MSVC 2019+)
- Windows SDK

### Build Commands

**MinGW:**

```bash
g++ -std=c++20 -Wall -Wextra -I include -I src src/*.cpp src/modules/*.cpp -o bin/wshell.exe -static -lpsapi -liphlpapi -lws2_32
```

**MSVC:**

```bash
cl /std:c++20 /EHsc /I include /I src src\*.cpp src\modules\*.cpp /Fe:bin\wshell.exe psapi.lib iphlpapi.lib ws2_32.lib
```

## Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Ideas for Contribution

- [ ] Script execution (.wsh files)
- [ ] Configuration file support
- [ ] Syntax highlighting
- [ ] Plugin system
- [ ] SSH client
- [ ] Background jobs
- [ ] Tab completion for command arguments
- [ ] Persistent history across sessions

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2025 Waleed

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Author

**Waleed** - Software Developer

## Acknowledgments

- Microsoft Windows API Documentation
- MSYS2 Project for MinGW-w64
- The C++ community for modern C++ practices

---

<p align="center">
  Made with ❤️ in C++
</p>
