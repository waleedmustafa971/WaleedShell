#include "registry.hpp"

namespace WaleedShell {

HKEY RegistryManager::parseRootKey(const std::string& key) {
    std::string root = key.substr(0, key.find('\\'));
    std::transform(root.begin(), root.end(), root.begin(), ::toupper);
    
    if (root == "HKEY_LOCAL_MACHINE" || root == "HKLM") return HKEY_LOCAL_MACHINE;
    if (root == "HKEY_CURRENT_USER" || root == "HKCU") return HKEY_CURRENT_USER;
    if (root == "HKEY_CLASSES_ROOT" || root == "HKCR") return HKEY_CLASSES_ROOT;
    if (root == "HKEY_USERS" || root == "HKU") return HKEY_USERS;
    if (root == "HKEY_CURRENT_CONFIG" || root == "HKCC") return HKEY_CURRENT_CONFIG;
    
    return NULL;
}

std::string RegistryManager::parseSubKey(const std::string& key) {
    size_t pos = key.find('\\');
    if (pos == std::string::npos) return "";
    return key.substr(pos + 1);
}

bool RegistryManager::keyExists(const std::string& key) {
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return false;
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    LONG result = RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

std::vector<std::string> RegistryManager::enumSubKeys(const std::string& key) {
    std::vector<std::string> subKeys;
    
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return subKeys;
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    if (RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return subKeys;
    }
    
    char name[256];
    DWORD index = 0;
    DWORD nameLen = sizeof(name);
    
    while (RegEnumKeyExA(hKey, index++, name, &nameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        subKeys.push_back(name);
        nameLen = sizeof(name);
    }
    
    RegCloseKey(hKey);
    return subKeys;
}

std::vector<std::pair<std::string, std::string>> RegistryManager::enumValues(const std::string& key) {
    std::vector<std::pair<std::string, std::string>> values;
    
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return values;
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    if (RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return values;
    }
    
    char name[256];
    BYTE data[1024];
    DWORD index = 0;
    DWORD nameLen = sizeof(name);
    DWORD dataLen = sizeof(data);
    DWORD type;
    
    while (RegEnumValueA(hKey, index++, name, &nameLen, NULL, &type, data, &dataLen) == ERROR_SUCCESS) {
        std::string valueStr;
        
        switch (type) {
            case REG_SZ:
            case REG_EXPAND_SZ:
                valueStr = std::string(reinterpret_cast<char*>(data));
                break;
            case REG_DWORD:
                valueStr = std::to_string(*reinterpret_cast<DWORD*>(data));
                break;
            case REG_QWORD:
                valueStr = std::to_string(*reinterpret_cast<ULONGLONG*>(data));
                break;
            default:
                valueStr = "(binary data)";
                break;
        }
        
        values.push_back({name, valueStr});
        nameLen = sizeof(name);
        dataLen = sizeof(data);
    }
    
    RegCloseKey(hKey);
    return values;
}

std::string RegistryManager::readString(const std::string& key, const std::string& valueName) {
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return "";
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    if (RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return "";
    }
    
    char data[1024];
    DWORD dataLen = sizeof(data);
    DWORD type;
    
    LONG result = RegQueryValueExA(hKey, valueName.c_str(), NULL, &type, reinterpret_cast<BYTE*>(data), &dataLen);
    RegCloseKey(hKey);
    
    if (result == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ)) {
        return std::string(data);
    }
    return "";
}

DWORD RegistryManager::readDword(const std::string& key, const std::string& valueName) {
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return 0;
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    if (RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return 0;
    }
    
    DWORD data;
    DWORD dataLen = sizeof(data);
    DWORD type;
    
    LONG result = RegQueryValueExA(hKey, valueName.c_str(), NULL, &type, reinterpret_cast<BYTE*>(&data), &dataLen);
    RegCloseKey(hKey);
    
    if (result == ERROR_SUCCESS && type == REG_DWORD) {
        return data;
    }
    return 0;
}

bool RegistryManager::writeString(const std::string& key, const std::string& valueName, const std::string& value) {
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return false;
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    if (RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return false;
    }
    
    LONG result = RegSetValueExA(hKey, valueName.c_str(), 0, REG_SZ, 
        reinterpret_cast<const BYTE*>(value.c_str()), static_cast<DWORD>(value.size() + 1));
    RegCloseKey(hKey);
    
    return result == ERROR_SUCCESS;
}

bool RegistryManager::writeDword(const std::string& key, const std::string& valueName, DWORD value) {
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return false;
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    if (RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return false;
    }
    
    LONG result = RegSetValueExA(hKey, valueName.c_str(), 0, REG_DWORD, 
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    RegCloseKey(hKey);
    
    return result == ERROR_SUCCESS;
}

bool RegistryManager::createKey(const std::string& key) {
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return false;
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    DWORD disposition;
    
    LONG result = RegCreateKeyExA(hRoot, subKey.c_str(), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &disposition);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool RegistryManager::deleteKey(const std::string& key) {
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return false;
    
    std::string subKey = parseSubKey(key);
    return RegDeleteKeyA(hRoot, subKey.c_str()) == ERROR_SUCCESS;
}

bool RegistryManager::deleteValue(const std::string& key, const std::string& valueName) {
    HKEY hRoot = parseRootKey(key);
    if (!hRoot) return false;
    
    std::string subKey = parseSubKey(key);
    HKEY hKey;
    if (RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return false;
    }
    
    LONG result = RegDeleteValueA(hKey, valueName.c_str());
    RegCloseKey(hKey);
    
    return result == ERROR_SUCCESS;
}

}