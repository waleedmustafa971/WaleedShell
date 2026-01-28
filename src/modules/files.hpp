#pragma once
#include "common.hpp"

namespace WaleedShell {

struct FileInfo {
    std::string name;
    std::string path;
    DWORD attributes;
    LARGE_INTEGER size;
    FILETIME created;
    FILETIME modified;
    bool isDirectory;
};

class FileManager {
public:
    std::vector<FileInfo> listDirectory(const std::string& path, bool recursive = false);
    bool copyFile(const std::string& src, const std::string& dst, bool overwrite = false);
    bool moveFile(const std::string& src, const std::string& dst);
    bool deleteFile(const std::string& path);
    bool createDirectory(const std::string& path);
    bool deleteDirectory(const std::string& path, bool recursive = false);
    bool fileExists(const std::string& path);
    FileInfo getFileInfo(const std::string& path);
    std::string formatSize(LARGE_INTEGER size);
    std::string formatTime(FILETIME ft);
    std::string readFile(const std::string& path);
    bool writeFile(const std::string& path, const std::string& content, bool append = false);
    std::vector<std::string> findFiles(const std::string& pattern);
};

}