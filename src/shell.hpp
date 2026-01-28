#pragma once
#include "common.hpp"
#include "parser.hpp"
#include "executor.hpp"
#include "input.hpp"
#include "modules/process.hpp"
#include "modules/files.hpp"
#include "modules/sysinfo.hpp"
#include "modules/registry.hpp"
#include "modules/network.hpp"
#include "modules/services.hpp"

namespace WaleedShell {

class Shell {
public:
    Shell();
    void run();
    std::unordered_map<std::string, std::string>& getAliases() { return m_aliases; }
    bool isBuiltin(const std::string& cmd);
    std::string executeBuiltinCapture(Command& cmd);
    
private:
    bool m_running;
    std::string m_currentDir;
    Parser m_parser;
    Executor m_executor;
    InputHandler m_input;
    std::unordered_map<std::string, std::string> m_aliases;
    
    ProcessManager m_processManager;
    FileManager m_fileManager;
    SystemInfoManager m_sysInfoManager;
    RegistryManager m_registryManager;
    NetworkManager m_networkManager;
    ServiceManager m_serviceManager;
    
    void printBanner();
    std::string getPrompt();
    void processCommand(const std::string& input);
    bool handleBuiltin(Command& cmd);
    std::string expandAliases(const std::string& input);
    std::string findExecutable(const std::string& program);
};

}