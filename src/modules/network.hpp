#pragma once
#include "../common.hpp"
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace WaleedShell {

struct AdapterInfo {
    std::string name;
    std::string description;
    std::string macAddress;
    std::string ipAddress;
    std::string subnet;
    std::string gateway;
    std::string dhcpServer;
    bool dhcpEnabled;
};

struct ConnectionInfo {
    std::string protocol;
    std::string localAddress;
    DWORD localPort;
    std::string remoteAddress;
    DWORD remotePort;
    std::string state;
    DWORD pid;
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    
    std::vector<AdapterInfo> getAdapters();
    std::vector<ConnectionInfo> getTcpConnections();
    std::vector<ConnectionInfo> getUdpConnections();
    bool ping(const std::string& host, DWORD timeout = 1000);
    std::string resolve(const std::string& hostname);
    void printAdapters();
    void printConnections();
    
private:
    bool m_wsaInitialized;
    std::string formatMac(BYTE* addr, DWORD len);
    std::string tcpStateToString(DWORD state);
};

}