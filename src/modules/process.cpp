#include "process.hpp"

namespace WaleedShell {

std::string ProcessManager::formatSize(SIZE_T bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    while (size >= 1024 && unit < 3) {
        size /= 1024;
        unit++;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << size << " " << units[unit];
    return ss.str();
}

std::vector<ProcessInfo> ProcessManager::listProcesses() {
    std::vector<ProcessInfo> processes;
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return processes;
    
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    
    if (Process32First(snapshot, &pe)) {
        do {
            ProcessInfo info;
            info.pid = pe.th32ProcessID;
            info.parentPid = pe.th32ParentProcessID;
            info.name = pe.szExeFile;
            info.threadCount = pe.cntThreads;
            info.memoryUsage = 0;
            
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
            if (hProcess) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    info.memoryUsage = pmc.WorkingSetSize;
                }
                
                char path[MAX_PATH];
                if (GetModuleFileNameExA(hProcess, NULL, path, MAX_PATH)) {
                    info.path = path;
                }
                CloseHandle(hProcess);
            }
            
            processes.push_back(info);
        } while (Process32Next(snapshot, &pe));
    }
    
    CloseHandle(snapshot);
    return processes;
}

bool ProcessManager::killProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) return false;
    
    BOOL result = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return result != 0;
}

bool ProcessManager::killProcessByName(const std::string& name) {
    auto processes = listProcesses();
    bool killed = false;
    
    for (const auto& proc : processes) {
        std::string procName = proc.name;
        std::string targetName = name;
        std::transform(procName.begin(), procName.end(), procName.begin(), ::tolower);
        std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::tolower);
        
        if (procName == targetName || procName == targetName + ".exe") {
            if (killProcess(proc.pid)) {
                killed = true;
            }
        }
    }
    return killed;
}

DWORD ProcessManager::startProcess(const std::string& command, bool wait) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    if (!CreateProcessA(NULL, const_cast<char*>(command.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return 0;
    }
    
    if (wait) {
        WaitForSingleObject(pi.hProcess, INFINITE);
    }
    
    DWORD pid = pi.dwProcessId;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return pid;
}

ProcessInfo ProcessManager::getProcessInfo(DWORD pid) {
    ProcessInfo info = {};
    info.pid = pid;
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        
        if (Process32First(snapshot, &pe)) {
            do {
                if (pe.th32ProcessID == pid) {
                    info.name = pe.szExeFile;
                    info.parentPid = pe.th32ParentProcessID;
                    info.threadCount = pe.cntThreads;
                    break;
                }
            } while (Process32Next(snapshot, &pe));
        }
        CloseHandle(snapshot);
    }
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            info.memoryUsage = pmc.WorkingSetSize;
        }
        
        char path[MAX_PATH];
        if (GetModuleFileNameExA(hProcess, NULL, path, MAX_PATH)) {
            info.path = path;
        }
        CloseHandle(hProcess);
    }
    
    return info;
}

}