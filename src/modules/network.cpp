#include "network.hpp"
#include <icmpapi.h>

namespace WaleedShell {

NetworkManager::NetworkManager() : m_wsaInitialized(false) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
        m_wsaInitialized = true;
    }
}

NetworkManager::~NetworkManager() {
    if (m_wsaInitialized) {
        WSACleanup();
    }
}

std::string NetworkManager::formatMac(BYTE* addr, DWORD len) {
    std::ostringstream ss;
    for (DWORD i = 0; i < len; ++i) {
        if (i > 0) ss << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(addr[i]);
    }
    return ss.str();
}

std::string NetworkManager::tcpStateToString(DWORD state) {
    switch (state) {
        case MIB_TCP_STATE_CLOSED: return "CLOSED";
        case MIB_TCP_STATE_LISTEN: return "LISTENING";
        case MIB_TCP_STATE_SYN_SENT: return "SYN_SENT";
        case MIB_TCP_STATE_SYN_RCVD: return "SYN_RCVD";
        case MIB_TCP_STATE_ESTAB: return "ESTABLISHED";
        case MIB_TCP_STATE_FIN_WAIT1: return "FIN_WAIT1";
        case MIB_TCP_STATE_FIN_WAIT2: return "FIN_WAIT2";
        case MIB_TCP_STATE_CLOSE_WAIT: return "CLOSE_WAIT";
        case MIB_TCP_STATE_CLOSING: return "CLOSING";
        case MIB_TCP_STATE_LAST_ACK: return "LAST_ACK";
        case MIB_TCP_STATE_TIME_WAIT: return "TIME_WAIT";
        default: return "UNKNOWN";
    }
}

std::vector<AdapterInfo> NetworkManager::getAdapters() {
    std::vector<AdapterInfo> adapters;
    
    ULONG bufLen = 0;
    GetAdaptersInfo(NULL, &bufLen);
    
    std::vector<BYTE> buffer(bufLen);
    PIP_ADAPTER_INFO pAdapter = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());
    
    if (GetAdaptersInfo(pAdapter, &bufLen) == NO_ERROR) {
        while (pAdapter) {
            AdapterInfo info;
            info.name = pAdapter->AdapterName;
            info.description = pAdapter->Description;
            info.macAddress = formatMac(pAdapter->Address, pAdapter->AddressLength);
            info.ipAddress = pAdapter->IpAddressList.IpAddress.String;
            info.subnet = pAdapter->IpAddressList.IpMask.String;
            info.gateway = pAdapter->GatewayList.IpAddress.String;
            info.dhcpEnabled = pAdapter->DhcpEnabled != 0;
            if (info.dhcpEnabled) {
                info.dhcpServer = pAdapter->DhcpServer.IpAddress.String;
            }
            
            adapters.push_back(info);
            pAdapter = pAdapter->Next;
        }
    }
    
    return adapters;
}

std::vector<ConnectionInfo> NetworkManager::getTcpConnections() {
    std::vector<ConnectionInfo> connections;
    
    ULONG size = 0;
    GetExtendedTcpTable(NULL, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    
    std::vector<BYTE> buffer(size);
    PMIB_TCPTABLE_OWNER_PID pTcpTable = reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(buffer.data());
    
    if (GetExtendedTcpTable(pTcpTable, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pTcpTable->dwNumEntries; ++i) {
            MIB_TCPROW_OWNER_PID& row = pTcpTable->table[i];
            
            ConnectionInfo info;
            info.protocol = "TCP";
            
            in_addr localAddr, remoteAddr;
            localAddr.S_un.S_addr = row.dwLocalAddr;
            remoteAddr.S_un.S_addr = row.dwRemoteAddr;
            
            info.localAddress = inet_ntoa(localAddr);
            info.localPort = ntohs(static_cast<u_short>(row.dwLocalPort));
            info.remoteAddress = inet_ntoa(remoteAddr);
            info.remotePort = ntohs(static_cast<u_short>(row.dwRemotePort));
            info.state = tcpStateToString(row.dwState);
            info.pid = row.dwOwningPid;
            
            connections.push_back(info);
        }
    }
    
    return connections;
}

std::vector<ConnectionInfo> NetworkManager::getUdpConnections() {
    std::vector<ConnectionInfo> connections;
    
    ULONG size = 0;
    GetExtendedUdpTable(NULL, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
    
    std::vector<BYTE> buffer(size);
    PMIB_UDPTABLE_OWNER_PID pUdpTable = reinterpret_cast<PMIB_UDPTABLE_OWNER_PID>(buffer.data());
    
    if (GetExtendedUdpTable(pUdpTable, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pUdpTable->dwNumEntries; ++i) {
            MIB_UDPROW_OWNER_PID& row = pUdpTable->table[i];
            
            ConnectionInfo info;
            info.protocol = "UDP";
            
            in_addr localAddr;
            localAddr.S_un.S_addr = row.dwLocalAddr;
            
            info.localAddress = inet_ntoa(localAddr);
            info.localPort = ntohs(static_cast<u_short>(row.dwLocalPort));
            info.remoteAddress = "*";
            info.remotePort = 0;
            info.state = "";
            info.pid = row.dwOwningPid;
            
            connections.push_back(info);
        }
    }
    
    return connections;
}

bool NetworkManager::ping(const std::string& host, DWORD timeout) {
    HANDLE hIcmp = IcmpCreateFile();
    if (hIcmp == INVALID_HANDLE_VALUE) return false;
    
    std::string ip = resolve(host);
    if (ip.empty()) {
        IcmpCloseHandle(hIcmp);
        return false;
    }
    
    DWORD addr = inet_addr(ip.c_str());
    
    char sendData[] = "WaleedShell Ping";
    DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData) + 8;
    std::vector<BYTE> replyBuffer(replySize);
    
    DWORD replies = IcmpSendEcho(hIcmp, addr, sendData, sizeof(sendData), NULL, 
        replyBuffer.data(), replySize, timeout);
    
    IcmpCloseHandle(hIcmp);
    
    if (replies > 0) {
        PICMP_ECHO_REPLY pReply = reinterpret_cast<PICMP_ECHO_REPLY>(replyBuffer.data());
        std::cout << "Reply from " << ip << ": bytes=" << pReply->DataSize 
                  << " time=" << pReply->RoundTripTime << "ms TTL=" << (int)pReply->Options.Ttl << "\n";
        return true;
    }
    
    std::cout << "Request timed out.\n";
    return false;
}

std::string NetworkManager::resolve(const std::string& hostname) {
    if (!m_wsaInitialized) return "";
    
    DWORD addr = inet_addr(hostname.c_str());
    if (addr != INADDR_NONE) {
        return hostname;
    }
    
    struct hostent* host = gethostbyname(hostname.c_str());
    if (host && host->h_addr_list[0]) {
        in_addr addr;
        addr.S_un.S_addr = *reinterpret_cast<DWORD*>(host->h_addr_list[0]);
        return inet_ntoa(addr);
    }
    
    return "";
}

void NetworkManager::printAdapters() {
    auto adapters = getAdapters();
    
    std::cout << "Network Adapters\n";
    std::cout << "================\n";
    
    for (const auto& adapter : adapters) {
        std::cout << adapter.description << "\n";
        std::cout << "    MAC:     " << adapter.macAddress << "\n";
        std::cout << "    IP:      " << adapter.ipAddress << "\n";
        std::cout << "    Subnet:  " << adapter.subnet << "\n";
        std::cout << "    Gateway: " << adapter.gateway << "\n";
        if (adapter.dhcpEnabled) {
            std::cout << "    DHCP:    " << adapter.dhcpServer << "\n";
        }
        std::cout << "\n";
    }
}

void NetworkManager::printConnections() {
    auto tcp = getTcpConnections();
    auto udp = getUdpConnections();
    
    std::cout << "Active Connections\n";
    std::cout << "==================\n";
    std::cout << std::left << std::setw(8) << "Proto" 
              << std::setw(24) << "Local Address" 
              << std::setw(24) << "Remote Address"
              << std::setw(16) << "State"
              << "PID\n";
    
    for (const auto& conn : tcp) {
        std::ostringstream local, remote;
        local << conn.localAddress << ":" << conn.localPort;
        remote << conn.remoteAddress << ":" << conn.remotePort;
        
        std::cout << std::left << std::setw(8) << conn.protocol
                  << std::setw(24) << local.str()
                  << std::setw(24) << remote.str()
                  << std::setw(16) << conn.state
                  << conn.pid << "\n";
    }
    
    for (const auto& conn : udp) {
        std::ostringstream local;
        local << conn.localAddress << ":" << conn.localPort;
        
        std::cout << std::left << std::setw(8) << conn.protocol
                  << std::setw(24) << local.str()
                  << std::setw(24) << "*:*"
                  << std::setw(16) << ""
                  << conn.pid << "\n";
    }
}

}