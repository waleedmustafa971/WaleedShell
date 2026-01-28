#pragma once
#include "common.hpp"
#include "parser.hpp"

namespace WaleedShell {

class Shell;

class Executor {
public:
    Executor() : m_shell(nullptr) {}
    void setShell(Shell* shell) { m_shell = shell; }
    int execute(Pipeline& pipeline);
    
private:
    Shell* m_shell;
    
    int executeSingle(Command& cmd);
    int executePipeline(Pipeline& pipeline);
    std::string buildCommandLine(Command& cmd);
    std::string findExecutable(const std::string& program);
    bool isCmdBuiltin(const std::string& program);
};

}