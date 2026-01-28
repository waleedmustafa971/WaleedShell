#include "files.hpp"

namespace WaleedShell {

std::string FileManager::formatSize(LARGE_INTEGER size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double sz = static_cast<double>(size.QuadPart);
    while (sz >= 1024 && unit < 4) {
        sz /= 1024;
        unit++;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << sz << " " << units[unit];
    return ss.str();
}

std::string FileManager::formatTime(FILETIME ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << st.wDay << "/"
       << std::setw(2) << st.wMonth << "/" << st.wYear << " "
       << std::setw(2) << st.wHour << ":" << std::setw(2) << st.wMinute;
    return ss.str();
}

std::vector<FileInfo> FileManager::listDirectory(const std::string& path, bool recursive) {
    std::vector<FileInfo> files;
    std::string searchPath = path + "\\*";
    
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
    
    if (hFind == INVALID_HANDLE_VALUE) return files;
    
    do {
        std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        
        FileInfo info;
        info.name = name;
        info.path = path + "\\" + name;
        info.attributes = fd.dwFileAttributes;
        info.size.LowPart = fd.nFileSizeLow;
        info.size.HighPart = fd.nFileSizeHigh;
        info.created = fd.ftCreationTime;
        info.modified = fd.ftLastWriteTime;
        info.isDirectory = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        
        files.push_back(info);
        
        if (recursive && info.isDirectory) {
            auto subFiles = listDirectory(info.path, true);
            files.insert(files.end(), subFiles.begin(), subFiles.end());
        }
    } while (FindNextFileA(hFind, &fd));
    
    FindClose(hFind);
    return files;
}

bool FileManager::copyFile(const std::string& src, const std::string& dst, bool overwrite) {
    return CopyFileA(src.c_str(), dst.c_str(), !overwrite) != 0;
}

bool FileManager::moveFile(const std::string& src, const std::string& dst) {
    return MoveFileA(src.c_str(), dst.c_str()) != 0;
}

bool FileManager::deleteFile(const std::string& path) {
    return DeleteFileA(path.c_str()) != 0;
}

bool FileManager::createDirectory(const std::string& path) {
    return CreateDirectoryA(path.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool FileManager::deleteDirectory(const std::string& path, bool recursive) {
    if (recursive) {
        auto files = listDirectory(path, false);
        for (const auto& file : files) {
            if (file.isDirectory) {
                deleteDirectory(file.path, true);
            } else {
                deleteFile(file.path);
            }
        }
    }
    return RemoveDirectoryA(path.c_str()) != 0;
}

bool FileManager::fileExists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES;
}

FileInfo FileManager::getFileInfo(const std::string& path) {
    FileInfo info = {};
    info.path = path;
    
    size_t lastSlash = path.find_last_of("\\/");
    info.name = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
    
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        info.attributes = fd.dwFileAttributes;
        info.size.LowPart = fd.nFileSizeLow;
        info.size.HighPart = fd.nFileSizeHigh;
        info.created = fd.ftCreationTime;
        info.modified = fd.ftLastWriteTime;
        info.isDirectory = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        FindClose(hFind);
    }
    
    return info;
}

std::string FileManager::readFile(const std::string& path) {
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return "";
    
    LARGE_INTEGER size;
    GetFileSizeEx(hFile, &size);
    
    std::string content(static_cast<size_t>(size.QuadPart), '\0');
    DWORD read;
    ReadFile(hFile, &content[0], static_cast<DWORD>(size.QuadPart), &read, NULL);
    CloseHandle(hFile);
    
    return content;
}

bool FileManager::writeFile(const std::string& path, const std::string& content, bool append) {
    DWORD access = append ? FILE_APPEND_DATA : GENERIC_WRITE;
    DWORD creation = append ? OPEN_ALWAYS : CREATE_ALWAYS;
    
    HANDLE hFile = CreateFileA(path.c_str(), access, 0, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    
    DWORD written;
    BOOL result = WriteFile(hFile, content.c_str(), static_cast<DWORD>(content.size()), &written, NULL);
    CloseHandle(hFile);
    
    return result != 0;
}

std::vector<std::string> FileManager::findFiles(const std::string& pattern) {
    std::vector<std::string> results;
    
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                results.push_back(fd.cFileName);
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
    
    return results;
}

}