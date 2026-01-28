#pragma once
#include "common.hpp"

namespace WaleedShell {

struct SystemInfo {
    std::string computerName;
    std::string userName;
    std::string osVersion;
    DWORD processorCount;
    std::string processorArch;
    DWORDLONG totalPhysicalMemory;
    DWORDLONG availablePhysicalMemory;
    DWORDLONG totalVirtualMemory;
    DWORDLONG availableVirtualMemory;
    DWORD memoryLoad;
};

struct DiskInfo {
    std::string drive;
    std::string label;
    std::string fileSystem;
    ULARGE_INTEGER totalSpace;
    ULARGE_INTEGER freeSpace;
    ULARGE_INTEGER usedSpace;
};

class SystemInfoManager {
public:
    SystemInfo getSystemInfo();
    std::vector<DiskInfo> getDiskInfo();
    std::string getUptime();
    std::string formatSize(ULONGLONG bytes);
    void printSystemInfo();
    void printDiskInfo();
    void printMemoryInfo();
};

}