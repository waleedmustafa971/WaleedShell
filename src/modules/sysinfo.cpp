#include "sysinfo.hpp"

namespace WaleedShell {

std::string SystemInfoManager::formatSize(ULONGLONG bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    while (size >= 1024 && unit < 4) {
        size /= 1024;
        unit++;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return ss.str();
}

SystemInfo SystemInfoManager::getSystemInfo() {
    SystemInfo info = {};
    
    char buffer[256];
    DWORD size = sizeof(buffer);
    
    if (GetComputerNameA(buffer, &size)) {
        info.computerName = buffer;
    }
    
    size = sizeof(buffer);
    if (GetUserNameA(buffer, &size)) {
        info.userName = buffer;
    }
    
    OSVERSIONINFOEXA osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    
    typedef LONG(WINAPI* RtlGetVersionPtr)(OSVERSIONINFOEXA*);
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (RtlGetVersion) {
            RtlGetVersion(&osvi);
            std::ostringstream ss;
            ss << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion 
               << " Build " << osvi.dwBuildNumber;
            info.osVersion = ss.str();
        }
    }
    
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    info.processorCount = si.dwNumberOfProcessors;
    
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: info.processorArch = "x64"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: info.processorArch = "ARM64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: info.processorArch = "x86"; break;
        default: info.processorArch = "Unknown"; break;
    }
    
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    if (GlobalMemoryStatusEx(&ms)) {
        info.totalPhysicalMemory = ms.ullTotalPhys;
        info.availablePhysicalMemory = ms.ullAvailPhys;
        info.totalVirtualMemory = ms.ullTotalVirtual;
        info.availableVirtualMemory = ms.ullAvailVirtual;
        info.memoryLoad = ms.dwMemoryLoad;
    }
    
    return info;
}

std::vector<DiskInfo> SystemInfoManager::getDiskInfo() {
    std::vector<DiskInfo> disks;
    
    DWORD drives = GetLogicalDrives();
    
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (drives & (1 << (letter - 'A'))) {
            std::string root = std::string(1, letter) + ":\\";
            
            UINT type = GetDriveTypeA(root.c_str());
            if (type == DRIVE_FIXED || type == DRIVE_REMOVABLE) {
                DiskInfo info;
                info.drive = root;
                
                char label[256] = {0};
                char fs[256] = {0};
                if (GetVolumeInformationA(root.c_str(), label, sizeof(label), NULL, NULL, NULL, fs, sizeof(fs))) {
                    info.label = label;
                    info.fileSystem = fs;
                }
                
                if (GetDiskFreeSpaceExA(root.c_str(), NULL, &info.totalSpace, &info.freeSpace)) {
                    info.usedSpace.QuadPart = info.totalSpace.QuadPart - info.freeSpace.QuadPart;
                }
                
                disks.push_back(info);
            }
        }
    }
    
    return disks;
}

std::string SystemInfoManager::getUptime() {
    ULONGLONG uptime = GetTickCount64();
    
    ULONGLONG seconds = uptime / 1000;
    ULONGLONG minutes = seconds / 60;
    ULONGLONG hours = minutes / 60;
    ULONGLONG days = hours / 24;
    
    std::ostringstream ss;
    if (days > 0) ss << days << "d ";
    ss << (hours % 24) << "h " << (minutes % 60) << "m " << (seconds % 60) << "s";
    return ss.str();
}

void SystemInfoManager::printSystemInfo() {
    auto info = getSystemInfo();
    
    std::cout << "System Information\n";
    std::cout << "==================\n";
    std::cout << "Computer Name:  " << info.computerName << "\n";
    std::cout << "User Name:      " << info.userName << "\n";
    std::cout << "OS Version:     " << info.osVersion << "\n";
    std::cout << "Architecture:   " << info.processorArch << "\n";
    std::cout << "Processors:     " << info.processorCount << "\n";
    std::cout << "Uptime:         " << getUptime() << "\n";
}

void SystemInfoManager::printMemoryInfo() {
    auto info = getSystemInfo();
    
    std::cout << "Memory Information\n";
    std::cout << "==================\n";
    std::cout << "Memory Load:    " << info.memoryLoad << "%\n";
    std::cout << "Physical Total: " << formatSize(info.totalPhysicalMemory) << "\n";
    std::cout << "Physical Free:  " << formatSize(info.availablePhysicalMemory) << "\n";
    std::cout << "Physical Used:  " << formatSize(info.totalPhysicalMemory - info.availablePhysicalMemory) << "\n";
}

void SystemInfoManager::printDiskInfo() {
    auto disks = getDiskInfo();
    
    std::cout << "Disk Information\n";
    std::cout << "================\n";
    for (const auto& disk : disks) {
        double usedPercent = 0;
        if (disk.totalSpace.QuadPart > 0) {
            usedPercent = (double)disk.usedSpace.QuadPart / disk.totalSpace.QuadPart * 100;
        }
        
        std::cout << disk.drive << " ";
        if (!disk.label.empty()) std::cout << "[" << disk.label << "] ";
        std::cout << disk.fileSystem << "\n";
        std::cout << "    Total: " << formatSize(disk.totalSpace.QuadPart) 
                  << "  Used: " << formatSize(disk.usedSpace.QuadPart) 
                  << " (" << std::fixed << std::setprecision(1) << usedPercent << "%)"
                  << "  Free: " << formatSize(disk.freeSpace.QuadPart) << "\n";
    }
}

}