#include "shell.hpp"

namespace WaleedShell {

Shell::Shell() : m_running(true) {
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    m_currentDir = buffer;
    m_executor.setShell(this);
}

void Shell::printBanner() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════╗\n";
    std::cout << "║         " << SHELL_NAME << " v" << VERSION << "            ║\n";
    std::cout << "║     Type 'help' for commands          ║\n";
    std::cout << "╚═══════════════════════════════════════╝\n";
    std::cout << "\n";
}

std::string Shell::getPrompt() {
    return m_currentDir + "\n" + PROMPT;
}

std::string Shell::findExecutable(const std::string& program) {
    std::vector<std::string> extensions = {"", ".exe", ".cmd", ".bat", ".com"};
    
    char pathEnv[32767];
    GetEnvironmentVariableA("PATH", pathEnv, 32767);
    
    std::vector<std::string> paths;
    paths.push_back(".");
    
    std::stringstream ss(pathEnv);
    std::string path;
    while (std::getline(ss, path, ';')) {
        if (!path.empty()) paths.push_back(path);
    }
    
    for (const auto& dir : paths) {
        for (const auto& ext : extensions) {
            std::string fullPath = dir + "\\" + program + ext;
            if (GetFileAttributesA(fullPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                return fullPath;
            }
        }
    }
    return "";
}

std::string Shell::expandAliases(const std::string& input) {
    std::string result = input;
    size_t spacePos = input.find(' ');
    std::string firstWord = (spacePos == std::string::npos) ? input : input.substr(0, spacePos);
    
    auto it = m_aliases.find(firstWord);
    if (it != m_aliases.end()) {
        result = it->second;
        if (spacePos != std::string::npos) {
            result += input.substr(spacePos);
        }
    }
    return result;
}

bool Shell::isBuiltin(const std::string& cmd) {
    static std::vector<std::string> builtins = {
        "exit", "quit", "help", "clear", "cls", "cd", "pwd",
        "history", "alias", "unalias", "which", "env", "export",
        "ps", "kill", "start", "pinfo",
        "ls", "cat", "touch", "rm", "mkdir", "rmdir", "cp", "mv", "find", "finfo",
        "sysinfo", "meminfo", "diskinfo", "uptime",
        "reg",
        "netstat", "adapters", "ping", "resolve",
        "services", "svc"
    };
    return std::find(builtins.begin(), builtins.end(), cmd) != builtins.end();
}

std::string Shell::executeBuiltinCapture(Command& cmd) {
    std::stringstream ss;
    
    if (cmd.program == "pwd") {
        ss << m_currentDir << "\n";
    }
    else if (cmd.program == "history") {
        auto& history = m_input.getHistory();
        for (size_t i = 0; i < history.size(); ++i) {
            ss << "  " << (i + 1) << "  " << history[i] << "\n";
        }
    }
    else if (cmd.program == "alias") {
        if (cmd.args.empty()) {
            for (const auto& [name, value] : m_aliases) {
                ss << "  " << name << "='" << value << "'\n";
            }
        }
    }
    else if (cmd.program == "which") {
        if (!cmd.args.empty()) {
            std::string path = findExecutable(cmd.args[0]);
            if (!path.empty()) {
                ss << path << "\n";
            } else {
                ss << cmd.args[0] << " not found\n";
            }
        }
    }
    else if (cmd.program == "env") {
        char* env = GetEnvironmentStringsA();
        if (env) {
            char* ptr = env;
            while (*ptr) {
                ss << ptr << "\n";
                ptr += strlen(ptr) + 1;
            }
            FreeEnvironmentStringsA(env);
        }
    }
    else if (cmd.program == "export") {
        if (!cmd.args.empty()) {
            std::string arg = cmd.args[0];
            size_t eqPos = arg.find('=');
            if (eqPos == std::string::npos) {
                char buffer[32767];
                if (GetEnvironmentVariableA(arg.c_str(), buffer, 32767)) {
                    ss << arg << "=" << buffer << "\n";
                }
            }
        }
    }
    else if (cmd.program == "help") {
        ss << "Built-in commands:\n";
        ss << "  help, exit, clear, cd, pwd, history\n";
        ss << "  alias, unalias, which, env, export\n";
    }
    
    return ss.str();
}

bool Shell::handleBuiltin(Command& cmd) {
    if (cmd.program == "exit" || cmd.program == "quit") {
        m_running = false;
        std::cout << "Goodbye!\n";
        return true;
    }
    
    if (cmd.program == "help") {
        std::cout << "WaleedShell Commands\n";
        std::cout << "====================\n\n";

        std::cout << "General:\n";
        std::cout << "  help              - Show this message\n";
        std::cout << "  exit              - Exit the shell\n";
        std::cout << "  clear/cls         - Clear screen\n";
        std::cout << "  cd <dir>          - Change directory\n";
        std::cout << "  pwd               - Print working directory\n";
        std::cout << "  history           - Command history\n";
        std::cout << "  alias/unalias     - Manage aliases\n";
        std::cout << "  which <cmd>       - Find executable\n";
        std::cout << "  env/export        - Environment variables\n\n";

        std::cout << "Process:\n";
        std::cout << "  ps                - List processes\n";
        std::cout << "  kill <pid|name>   - Terminate process\n";
        std::cout << "  start <cmd>       - Start new process\n";
        std::cout << "  pinfo <pid>       - Process details\n\n";

        std::cout << "Files:\n";
        std::cout << "  ls [path]         - List directory\n";
        std::cout << "  cat <file>        - Display file\n";
        std::cout << "  touch <file>      - Create file\n";
        std::cout << "  rm <file>         - Delete file\n";
        std::cout << "  cp <src> <dst>    - Copy file\n";
        std::cout << "  mv <src> <dst>    - Move file\n";
        std::cout << "  mkdir <dir>       - Create directory\n";
        std::cout << "  rmdir <dir> [-r]  - Delete directory\n";
        std::cout << "  find <pattern>    - Find files\n";
        std::cout << "  finfo <file>      - File details\n\n";

        std::cout << "System:\n";
        std::cout << "  sysinfo           - System information\n";
        std::cout << "  meminfo           - Memory information\n";
        std::cout << "  diskinfo          - Disk information\n";
        std::cout << "  uptime            - System uptime\n\n";

        std::cout << "Registry:\n";
        std::cout << "  reg query <key>   - Query registry\n";
        std::cout << "  reg add <k> <v> <d> - Set value\n";
        std::cout << "  reg delete <k> [v] - Delete key/value\n\n";

        std::cout << "Network:\n";
        std::cout << "  adapters          - Network adapters\n";
        std::cout << "  netstat           - Connections\n";
        std::cout << "  ping <host>       - Ping host\n";
        std::cout << "  resolve <host>    - DNS lookup\n\n";

        std::cout << "Services:\n";
        std::cout << "  services [-r]     - List services\n";
        std::cout << "  svc start <name>  - Start service\n";
        std::cout << "  svc stop <name>   - Stop service\n";
        std::cout << "  svc restart <name>- Restart service\n";
        std::cout << "  svc info <name>   - Service details\n";
        return true;
    }
    
    if (cmd.program == "clear" || cmd.program == "cls") {
        system("cls");
        return true;
    }
    
    if (cmd.program == "cd") {
        if (cmd.args.empty()) {
            std::cout << m_currentDir << "\n";
        } else {
            std::string target = cmd.args[0];
            if (SetCurrentDirectoryA(target.c_str())) {
                char buffer[MAX_PATH];
                GetCurrentDirectoryA(MAX_PATH, buffer);
                m_currentDir = buffer;
            } else {
                std::cerr << "Error: Cannot change to directory '" << cmd.args[0] << "'\n";
            }
        }
        return true;
    }

    // Process commands
    if (cmd.program == "ps") {
        auto processes = m_processManager.listProcesses();
        std::cout << std::left << std::setw(8) << "PID" 
                  << std::setw(32) << "Name"
                  << std::setw(12) << "Memory"
                  << "Threads\n";
        std::cout << std::string(60, '-') << "\n";
        for (const auto& proc : processes) {
            std::cout << std::left << std::setw(8) << proc.pid
                      << std::setw(32) << proc.name.substr(0, 31)
                      << std::setw(12) << m_processManager.formatSize(proc.memoryUsage)
                      << proc.threadCount << "\n";
        }
        return true;
    }
    
    if (cmd.program == "kill") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: kill <pid|name>\n";
        } else {
            try {
                DWORD pid = std::stoul(cmd.args[0]);
                if (m_processManager.killProcess(pid)) {
                    std::cout << "Process " << pid << " terminated.\n";
                } else {
                    std::cerr << "Failed to terminate process " << pid << "\n";
                }
            } catch (...) {
                if (m_processManager.killProcessByName(cmd.args[0])) {
                    std::cout << "Process(es) '" << cmd.args[0] << "' terminated.\n";
                } else {
                    std::cerr << "Failed to terminate process '" << cmd.args[0] << "'\n";
                }
            }
        }
        return true;
    }
    
    if (cmd.program == "start") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: start <command>\n";
        } else {
            std::string cmdLine;
            for (const auto& arg : cmd.args) {
                if (!cmdLine.empty()) cmdLine += " ";
                cmdLine += arg;
            }
            DWORD pid = m_processManager.startProcess(cmdLine, false);
            if (pid) {
                std::cout << "Started process with PID: " << pid << "\n";
            } else {
                std::cerr << "Failed to start process.\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "pinfo") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: pinfo <pid>\n";
        } else {
            DWORD pid = std::stoul(cmd.args[0]);
            auto info = m_processManager.getProcessInfo(pid);
            std::cout << "PID:      " << info.pid << "\n";
            std::cout << "Name:     " << info.name << "\n";
            std::cout << "Path:     " << info.path << "\n";
            std::cout << "Memory:   " << m_processManager.formatSize(info.memoryUsage) << "\n";
            std::cout << "Threads:  " << info.threadCount << "\n";
        }
        return true;
    }
    
    // File commands
    if (cmd.program == "ls") {
        std::string path = cmd.args.empty() ? "." : cmd.args[0];
        auto files = m_fileManager.listDirectory(path);
        for (const auto& file : files) {
            std::cout << (file.isDirectory ? "[DIR]  " : "       ")
                      << std::left << std::setw(32) << file.name;
            if (!file.isDirectory) {
                std::cout << m_fileManager.formatSize(file.size);
            }
            std::cout << "\n";
        }
        return true;
    }
    
    if (cmd.program == "cat") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: cat <file>\n";
        } else {
            std::string content = m_fileManager.readFile(cmd.args[0]);
            if (!content.empty()) {
                std::cout << content;
                if (content.back() != '\n') std::cout << "\n";
            } else {
                std::cerr << "Error: Cannot read file '" << cmd.args[0] << "'\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "touch") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: touch <file>\n";
        } else {
            if (m_fileManager.writeFile(cmd.args[0], "", true)) {
                std::cout << "Created: " << cmd.args[0] << "\n";
            } else {
                std::cerr << "Error: Cannot create file.\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "rm") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: rm <file>\n";
        } else {
            if (m_fileManager.deleteFile(cmd.args[0])) {
                std::cout << "Deleted: " << cmd.args[0] << "\n";
            } else {
                std::cerr << "Error: Cannot delete file.\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "mkdir") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: mkdir <directory>\n";
        } else {
            if (m_fileManager.createDirectory(cmd.args[0])) {
                std::cout << "Created: " << cmd.args[0] << "\n";
            } else {
                std::cerr << "Error: Cannot create directory.\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "rmdir") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: rmdir <directory> [-r]\n";
        } else {
            bool recursive = cmd.args.size() > 1 && cmd.args[1] == "-r";
            if (m_fileManager.deleteDirectory(cmd.args[0], recursive)) {
                std::cout << "Deleted: " << cmd.args[0] << "\n";
            } else {
                std::cerr << "Error: Cannot delete directory.\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "cp") {
        if (cmd.args.size() < 2) {
            std::cerr << "Usage: cp <source> <destination>\n";
        } else {
            if (m_fileManager.copyFile(cmd.args[0], cmd.args[1], true)) {
                std::cout << "Copied: " << cmd.args[0] << " -> " << cmd.args[1] << "\n";
            } else {
                std::cerr << "Error: Cannot copy file.\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "mv") {
        if (cmd.args.size() < 2) {
            std::cerr << "Usage: mv <source> <destination>\n";
        } else {
            if (m_fileManager.moveFile(cmd.args[0], cmd.args[1])) {
                std::cout << "Moved: " << cmd.args[0] << " -> " << cmd.args[1] << "\n";
            } else {
                std::cerr << "Error: Cannot move file.\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "find") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: find <pattern>\n";
        } else {
            auto files = m_fileManager.findFiles(cmd.args[0]);
            for (const auto& f : files) {
                std::cout << f << "\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "finfo") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: finfo <file>\n";
        } else {
            auto info = m_fileManager.getFileInfo(cmd.args[0]);
            std::cout << "Name:     " << info.name << "\n";
            std::cout << "Path:     " << info.path << "\n";
            std::cout << "Size:     " << m_fileManager.formatSize(info.size) << "\n";
            std::cout << "Type:     " << (info.isDirectory ? "Directory" : "File") << "\n";
            std::cout << "Created:  " << m_fileManager.formatTime(info.created) << "\n";
            std::cout << "Modified: " << m_fileManager.formatTime(info.modified) << "\n";
        }
        return true;
    }
    
    // System info commands
    if (cmd.program == "sysinfo") {
        m_sysInfoManager.printSystemInfo();
        return true;
    }
    
    if (cmd.program == "meminfo") {
        m_sysInfoManager.printMemoryInfo();
        return true;
    }
    
    if (cmd.program == "diskinfo") {
        m_sysInfoManager.printDiskInfo();
        return true;
    }
    
    if (cmd.program == "uptime") {
        std::cout << "Uptime: " << m_sysInfoManager.getUptime() << "\n";
        return true;
    }
    
    // Registry commands
    if (cmd.program == "reg") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: reg <query|add|delete> <key> [value] [data]\n";
        } else {
            std::string action = cmd.args[0];
            if (action == "query" && cmd.args.size() >= 2) {
                std::string key = cmd.args[1];
                if (m_registryManager.keyExists(key)) {
                    auto values = m_registryManager.enumValues(key);
                    auto subkeys = m_registryManager.enumSubKeys(key);
                    
                    std::cout << key << "\n";
                    for (const auto& [name, value] : values) {
                        std::cout << "    " << (name.empty() ? "(Default)" : name) << " = " << value << "\n";
                    }
                    if (!subkeys.empty()) {
                        std::cout << "Subkeys:\n";
                        for (const auto& sk : subkeys) {
                            std::cout << "    " << sk << "\n";
                        }
                    }
                } else {
                    std::cerr << "Key not found.\n";
                }
            } else if (action == "add" && cmd.args.size() >= 4) {
                if (m_registryManager.writeString(cmd.args[1], cmd.args[2], cmd.args[3])) {
                    std::cout << "Value set.\n";
                } else {
                    std::cerr << "Failed to set value.\n";
                }
            } else if (action == "delete" && cmd.args.size() >= 2) {
                if (cmd.args.size() >= 3) {
                    if (m_registryManager.deleteValue(cmd.args[1], cmd.args[2])) {
                        std::cout << "Value deleted.\n";
                    } else {
                        std::cerr << "Failed to delete value.\n";
                    }
                } else {
                    if (m_registryManager.deleteKey(cmd.args[1])) {
                        std::cout << "Key deleted.\n";
                    } else {
                        std::cerr << "Failed to delete key.\n";
                    }
                }
            } else {
                std::cerr << "Usage: reg <query|add|delete> <key> [value] [data]\n";
            }
        }
        return true;
    }
    
    // Network commands
    if (cmd.program == "netstat") {
        m_networkManager.printConnections();
        return true;
    }
    
    if (cmd.program == "adapters") {
        m_networkManager.printAdapters();
        return true;
    }
    
    if (cmd.program == "ping") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: ping <host>\n";
        } else {
            int count = 4;
            for (int i = 0; i < count; ++i) {
                m_networkManager.ping(cmd.args[0]);
                if (i < count - 1) Sleep(1000);
            }
        }
        return true;
    }
    
    if (cmd.program == "resolve") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: resolve <hostname>\n";
        } else {
            std::string ip = m_networkManager.resolve(cmd.args[0]);
            if (!ip.empty()) {
                std::cout << cmd.args[0] << " -> " << ip << "\n";
            } else {
                std::cerr << "Cannot resolve hostname.\n";
            }
        }
        return true;
    }
    
    // Service commands
    if (cmd.program == "services") {
        bool runningOnly = !cmd.args.empty() && cmd.args[0] == "-r";
        m_serviceManager.printServices(runningOnly);
        return true;
    }
    
    if (cmd.program == "svc") {
        if (cmd.args.size() < 2) {
            std::cerr << "Usage: svc <start|stop|restart|info> <service>\n";
        } else {
            std::string action = cmd.args[0];
            std::string name = cmd.args[1];
            
            if (action == "start") {
                if (m_serviceManager.startService(name)) {
                    std::cout << "Service started.\n";
                } else {
                    std::cerr << "Failed to start service.\n";
                }
            } else if (action == "stop") {
                if (m_serviceManager.stopService(name)) {
                    std::cout << "Service stopped.\n";
                } else {
                    std::cerr << "Failed to stop service.\n";
                }
            } else if (action == "restart") {
                if (m_serviceManager.restartService(name)) {
                    std::cout << "Service restarted.\n";
                } else {
                    std::cerr << "Failed to restart service.\n";
                }
            } else if (action == "info") {
                auto info = m_serviceManager.getServiceInfo(name);
                std::cout << "Name:       " << info.name << "\n";
                std::cout << "Display:    " << info.displayName << "\n";
                std::cout << "State:      " << info.stateStr << "\n";
                std::cout << "Start Type: " << info.startTypeStr << "\n";
            } else {
                std::cerr << "Unknown action: " << action << "\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "pwd") {
        std::cout << m_currentDir << "\n";
        return true;
    }
    
    if (cmd.program == "history") {
        auto& history = m_input.getHistory();
        for (size_t i = 0; i < history.size(); ++i) {
            std::cout << "  " << (i + 1) << "  " << history[i] << "\n";
        }
        return true;
    }
    
    if (cmd.program == "alias") {
        if (cmd.args.empty()) {
            if (m_aliases.empty()) {
                std::cout << "No aliases defined.\n";
            } else {
                for (const auto& [name, value] : m_aliases) {
                    std::cout << "  " << name << "='" << value << "'\n";
                }
            }
        } else {
            std::string arg = cmd.args[0];
            for (size_t i = 1; i < cmd.args.size(); ++i) {
                arg += " " + cmd.args[i];
            }
            size_t eqPos = arg.find('=');
            if (eqPos == std::string::npos) {
                auto it = m_aliases.find(arg);
                if (it != m_aliases.end()) {
                    std::cout << "  " << arg << "='" << it->second << "'\n";
                } else {
                    std::cout << "Alias not found: " << arg << "\n";
                }
            } else {
                std::string name = arg.substr(0, eqPos);
                std::string value = arg.substr(eqPos + 1);
                if (!value.empty() && value.front() == '\'' && value.back() == '\'') {
                    value = value.substr(1, value.size() - 2);
                }
                if (!value.empty() && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.size() - 2);
                }
                m_aliases[name] = value;
                std::cout << "Alias created: " << name << "='" << value << "'\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "unalias") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: unalias <name>\n";
        } else {
            auto it = m_aliases.find(cmd.args[0]);
            if (it != m_aliases.end()) {
                m_aliases.erase(it);
                std::cout << "Alias removed: " << cmd.args[0] << "\n";
            } else {
                std::cout << "Alias not found: " << cmd.args[0] << "\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "which") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: which <command>\n";
        } else {
            std::string path = findExecutable(cmd.args[0]);
            if (!path.empty()) {
                std::cout << path << "\n";
            } else {
                std::cout << cmd.args[0] << " not found\n";
            }
        }
        return true;
    }
    
    if (cmd.program == "env") {
        char* env = GetEnvironmentStringsA();
        if (env) {
            char* ptr = env;
            while (*ptr) {
                std::cout << ptr << "\n";
                ptr += strlen(ptr) + 1;
            }
            FreeEnvironmentStringsA(env);
        }
        return true;
    }
    
    if (cmd.program == "export") {
        if (cmd.args.empty()) {
            std::cerr << "Usage: export NAME=VALUE\n";
        } else {
            std::string arg = cmd.args[0];
            for (size_t i = 1; i < cmd.args.size(); ++i) {
                arg += " " + cmd.args[i];
            }
            size_t eqPos = arg.find('=');
            if (eqPos == std::string::npos) {
                char buffer[32767];
                if (GetEnvironmentVariableA(arg.c_str(), buffer, 32767)) {
                    std::cout << arg << "=" << buffer << "\n";
                } else {
                    std::cout << arg << " is not set\n";
                }
            } else {
                std::string name = arg.substr(0, eqPos);
                std::string value = arg.substr(eqPos + 1);
                if (SetEnvironmentVariableA(name.c_str(), value.c_str())) {
                    std::cout << "Set " << name << "=" << value << "\n";
                } else {
                    std::cerr << "Error setting variable\n";
                }
            }
        }
        return true;
    }
    
    return false;
}

void Shell::processCommand(const std::string& input) {
    if (input.empty()) return;
    
    std::string expanded = expandAliases(input);
    Pipeline pipeline = m_parser.parse(expanded);
    
    if (!pipeline.isValid) {
        if (!pipeline.error.empty()) {
            std::cout << pipeline.error << "\n";
        }
        return;
    }
    
    Command& firstCmd = pipeline.commands[0];
    
    if (pipeline.commands.size() == 1 && handleBuiltin(firstCmd)) {
        return;
    }
    
    m_executor.execute(pipeline);
}

void Shell::run() {
    printBanner();
    
    while (m_running) {
        std::string input = m_input.readLine(getPrompt());
        processCommand(input);
    }
}

}