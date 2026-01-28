#include "services.hpp"

namespace WaleedShell {

std::string ServiceManager::stateToString(DWORD state) {
    switch (state) {
        case SERVICE_STOPPED: return "Stopped";
        case SERVICE_START_PENDING: return "Starting";
        case SERVICE_STOP_PENDING: return "Stopping";
        case SERVICE_RUNNING: return "Running";
        case SERVICE_CONTINUE_PENDING: return "Resuming";
        case SERVICE_PAUSE_PENDING: return "Pausing";
        case SERVICE_PAUSED: return "Paused";
        default: return "Unknown";
    }
}

std::string ServiceManager::startTypeToString(DWORD type) {
    switch (type) {
        case SERVICE_AUTO_START: return "Automatic";
        case SERVICE_BOOT_START: return "Boot";
        case SERVICE_DEMAND_START: return "Manual";
        case SERVICE_DISABLED: return "Disabled";
        case SERVICE_SYSTEM_START: return "System";
        default: return "Unknown";
    }
}

std::vector<ServiceInfo> ServiceManager::listServices() {
    std::vector<ServiceInfo> services;
    
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) return services;
    
    DWORD bytesNeeded = 0;
    DWORD servicesReturned = 0;
    DWORD resumeHandle = 0;
    
    EnumServicesStatusA(hSCM, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, 
        &bytesNeeded, &servicesReturned, &resumeHandle);
    
    std::vector<BYTE> buffer(bytesNeeded);
    LPENUM_SERVICE_STATUSA pServices = reinterpret_cast<LPENUM_SERVICE_STATUSA>(buffer.data());
    
    if (EnumServicesStatusA(hSCM, SERVICE_WIN32, SERVICE_STATE_ALL, pServices, 
        bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle)) {
        
        for (DWORD i = 0; i < servicesReturned; ++i) {
            ServiceInfo info;
            info.name = pServices[i].lpServiceName;
            info.displayName = pServices[i].lpDisplayName;
            info.state = pServices[i].ServiceStatus.dwCurrentState;
            info.stateStr = stateToString(info.state);
            
            SC_HANDLE hService = OpenServiceA(hSCM, info.name.c_str(), SERVICE_QUERY_CONFIG);
            if (hService) {
                DWORD needed;
                QueryServiceConfigA(hService, NULL, 0, &needed);
                std::vector<BYTE> configBuf(needed);
                LPQUERY_SERVICE_CONFIGA pConfig = reinterpret_cast<LPQUERY_SERVICE_CONFIGA>(configBuf.data());
                if (QueryServiceConfigA(hService, pConfig, needed, &needed)) {
                    info.startType = pConfig->dwStartType;
                    info.startTypeStr = startTypeToString(info.startType);
                }
                CloseServiceHandle(hService);
            }
            
            services.push_back(info);
        }
    }
    
    CloseServiceHandle(hSCM);
    return services;
}

ServiceInfo ServiceManager::getServiceInfo(const std::string& name) {
    ServiceInfo info = {};
    info.name = name;
    
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) return info;
    
    SC_HANDLE hService = OpenServiceA(hSCM, name.c_str(), SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (hService) {
        SERVICE_STATUS status;
        if (QueryServiceStatus(hService, &status)) {
            info.state = status.dwCurrentState;
            info.stateStr = stateToString(info.state);
        }
        
        DWORD needed;
        QueryServiceConfigA(hService, NULL, 0, &needed);
        std::vector<BYTE> buffer(needed);
        LPQUERY_SERVICE_CONFIGA pConfig = reinterpret_cast<LPQUERY_SERVICE_CONFIGA>(buffer.data());
        if (QueryServiceConfigA(hService, pConfig, needed, &needed)) {
            info.displayName = pConfig->lpDisplayName;
            info.startType = pConfig->dwStartType;
            info.startTypeStr = startTypeToString(info.startType);
        }
        
        CloseServiceHandle(hService);
    }
    
    CloseServiceHandle(hSCM);
    return info;
}

bool ServiceManager::startService(const std::string& name) {
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) return false;
    
    SC_HANDLE hService = OpenServiceA(hSCM, name.c_str(), SERVICE_START);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return false;
    }
    
    BOOL result = StartServiceA(hService, 0, NULL);
    
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    
    return result != 0;
}

bool ServiceManager::stopService(const std::string& name) {
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) return false;
    
    SC_HANDLE hService = OpenServiceA(hSCM, name.c_str(), SERVICE_STOP);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return false;
    }
    
    SERVICE_STATUS status;
    BOOL result = ControlService(hService, SERVICE_CONTROL_STOP, &status);
    
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    
    return result != 0;
}

bool ServiceManager::restartService(const std::string& name) {
    if (!stopService(name)) return false;
    Sleep(1000);
    return startService(name);
}

bool ServiceManager::setStartType(const std::string& name, DWORD startType) {
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) return false;
    
    SC_HANDLE hService = OpenServiceA(hSCM, name.c_str(), SERVICE_CHANGE_CONFIG);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return false;
    }
    
    BOOL result = ChangeServiceConfigA(hService, SERVICE_NO_CHANGE, startType, 
        SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    
    return result != 0;
}

void ServiceManager::printServices(bool runningOnly) {
    auto services = listServices();
    
    std::cout << "Windows Services\n";
    std::cout << "================\n";
    std::cout << std::left << std::setw(36) << "Name" 
              << std::setw(12) << "State"
              << "Start Type\n";
    std::cout << std::string(60, '-') << "\n";
    
    for (const auto& svc : services) {
        if (runningOnly && svc.state != SERVICE_RUNNING) continue;
        
        std::cout << std::left << std::setw(36) << svc.name.substr(0, 35)
                  << std::setw(12) << svc.stateStr
                  << svc.startTypeStr << "\n";
    }
}

}