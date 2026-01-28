#pragma once
#include "common.hpp"

namespace WaleedShell {

struct ServiceInfo {
    std::string name;
    std::string displayName;
    DWORD state;
    DWORD startType;
    std::string stateStr;
    std::string startTypeStr;
};

class ServiceManager {
public:
    std::vector<ServiceInfo> listServices();
    ServiceInfo getServiceInfo(const std::string& name);
    bool startService(const std::string& name);
    bool stopService(const std::string& name);
    bool restartService(const std::string& name);
    bool setStartType(const std::string& name, DWORD startType);
    void printServices(bool runningOnly = false);
    
private:
    std::string stateToString(DWORD state);
    std::string startTypeToString(DWORD type);
};

}