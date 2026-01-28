#pragma once
#include "../common.hpp"
#include <tlhelp32.h>
#include <psapi.h>

namespace WaleedShell {

struct ProcessInfo {
    DWORD pid;
    DWORD parentPid;
    std::string name;
    std::string path;
    SIZE_T memoryUsage;
    DWORD threadCount;
};

class ProcessManager {
public:
    std::vector<ProcessInfo> listProcesses();
    bool killProcess(DWORD pid);
    bool killProcessByName(const std::string& name);
    DWORD startProcess(const std::string& command, bool wait = false);
    ProcessInfo getProcessInfo(DWORD pid);
    std::string formatSize(SIZE_T bytes);
};

}