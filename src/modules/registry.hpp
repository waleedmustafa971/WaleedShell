#pragma once
#include "../common.hpp"

namespace WaleedShell {

class RegistryManager {
public:
    HKEY parseRootKey(const std::string& key);
    std::string parseSubKey(const std::string& key);
    
    bool keyExists(const std::string& key);
    std::vector<std::string> enumSubKeys(const std::string& key);
    std::vector<std::pair<std::string, std::string>> enumValues(const std::string& key);
    
    std::string readString(const std::string& key, const std::string& valueName);
    DWORD readDword(const std::string& key, const std::string& valueName);
    
    bool writeString(const std::string& key, const std::string& valueName, const std::string& value);
    bool writeDword(const std::string& key, const std::string& valueName, DWORD value);
    
    bool createKey(const std::string& key);
    bool deleteKey(const std::string& key);
    bool deleteValue(const std::string& key, const std::string& valueName);
};

}