#include "input.hpp"

namespace WaleedShell {

InputHandler::InputHandler() : m_historyIndex(0) {
    m_hInput = GetStdHandle(STD_INPUT_HANDLE);
    m_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
}

void InputHandler::clearLine(size_t promptLen, size_t lineLen) {
    std::cout << '\r';
    for (size_t i = 0; i < promptLen + lineLen + 5; ++i) {
        std::cout << ' ';
    }
    std::cout << '\r';
}

void InputHandler::refreshLine(const std::string& prompt, const std::string& line, size_t cursorPos) {
    std::cout << '\r' << prompt << line;
    
    size_t totalLen = prompt.size() + line.size();
    for (size_t i = 0; i < 5; ++i) std::cout << ' ';
    for (size_t i = 0; i < 5; ++i) std::cout << '\b';
    
    if (cursorPos < line.size()) {
        size_t moveBack = line.size() - cursorPos;
        for (size_t i = 0; i < moveBack; ++i) {
            std::cout << '\b';
        }
    }
    std::cout.flush();
}

std::vector<std::string> InputHandler::getPathCompletions(const std::string& partial) {
    std::vector<std::string> completions;
    
    std::string searchPath = partial;
    std::string prefix;
    
    size_t lastSlash = partial.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        prefix = partial.substr(0, lastSlash + 1);
        searchPath = partial.substr(lastSlash + 1);
    }
    
    std::string searchDir = prefix.empty() ? "." : prefix;
    std::string pattern = searchDir + "\\*";
    
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = fd.cFileName;
            if (name == "." || name == "..") continue;
            
            std::string lowerName = name;
            std::string lowerSearch = searchPath;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
            
            if (lowerName.find(lowerSearch) == 0) {
                std::string completion = prefix + name;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    completion += "\\";
                }
                completions.push_back(completion);
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
    
    return completions;
}

std::vector<std::string> InputHandler::getExecutableCompletions(const std::string& partial) {
    std::vector<std::string> completions;
    std::vector<std::string> extensions = {".exe", ".cmd", ".bat", ".com"};
    
    char pathEnv[32767];
    GetEnvironmentVariableA("PATH", pathEnv, 32767);
    
    std::vector<std::string> paths;
    paths.push_back(".");
    
    std::stringstream ss(pathEnv);
    std::string path;
    while (std::getline(ss, path, ';')) {
        if (!path.empty()) {
            paths.push_back(path);
        }
    }
    
    std::string lowerPartial = partial;
    std::transform(lowerPartial.begin(), lowerPartial.end(), lowerPartial.begin(), ::tolower);
    
    for (const auto& dir : paths) {
        WIN32_FIND_DATAA fd;
        std::string pattern = dir + "\\*";
        HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::string name = fd.cFileName;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
                
                std::string lowerName = name;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                
                bool isExe = false;
                for (const auto& ext : extensions) {
                    if (lowerName.size() >= ext.size() && 
                        lowerName.substr(lowerName.size() - ext.size()) == ext) {
                        isExe = true;
                        break;
                    }
                }
                
                if (isExe && lowerName.find(lowerPartial) == 0) {
                    std::string baseName = name.substr(0, name.find_last_of('.'));
                    if (std::find(completions.begin(), completions.end(), baseName) == completions.end()) {
                        completions.push_back(baseName);
                    }
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
    }
    
    std::vector<std::string> builtins = {
        "dir", "echo", "type", "copy", "move", "del", "ren", "mkdir", "rmdir",
        "cd", "pwd", "clear", "cls", "exit", "help"
    };
    
    for (const auto& cmd : builtins) {
        if (cmd.find(lowerPartial) == 0) {
            if (std::find(completions.begin(), completions.end(), cmd) == completions.end()) {
                completions.push_back(cmd);
            }
        }
    }
    
    return completions;
}

std::vector<std::string> InputHandler::getCompletions(const std::string& partial) {
    if (partial.empty()) return {};
    
    if (partial.find(' ') == std::string::npos) {
        auto pathComps = getPathCompletions(partial);
        auto exeComps = getExecutableCompletions(partial);
        
        for (const auto& c : exeComps) {
            if (std::find(pathComps.begin(), pathComps.end(), c) == pathComps.end()) {
                pathComps.push_back(c);
            }
        }
        return pathComps;
    }
    
    return getPathCompletions(partial);
}

std::string InputHandler::getCommonPrefix(const std::vector<std::string>& strings) {
    if (strings.empty()) return "";
    if (strings.size() == 1) return strings[0];
    
    std::string prefix = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
        size_t j = 0;
        while (j < prefix.size() && j < strings[i].size() && 
               tolower(prefix[j]) == tolower(strings[i][j])) {
            ++j;
        }
        prefix = prefix.substr(0, j);
    }
    return prefix;
}

std::string InputHandler::readLine(const std::string& prompt) {
    std::string line;
    size_t cursorPos = 0;
    size_t historyNav = m_history.size();
    std::string savedLine;
    
    DWORD originalMode;
    GetConsoleMode(m_hInput, &originalMode);
    SetConsoleMode(m_hInput, ENABLE_PROCESSED_INPUT);
    
    std::cout << prompt;
    std::cout.flush();
    
    while (true) {
        INPUT_RECORD record;
        DWORD read;
        
        if (!ReadConsoleInputA(m_hInput, &record, 1, &read)) {
            break;
        }
        
        if (record.EventType != KEY_EVENT || !record.Event.KeyEvent.bKeyDown) {
            continue;
        }
        
        WORD vk = record.Event.KeyEvent.wVirtualKeyCode;
        char ch = record.Event.KeyEvent.uChar.AsciiChar;
        
        if (vk == VK_RETURN) {
            std::cout << '\n';
            break;
        }
        
        if (vk == VK_TAB) {
            size_t wordStart = line.find_last_of(" ", cursorPos - 1);
            wordStart = (wordStart == std::string::npos) ? 0 : wordStart + 1;
            std::string partial = line.substr(wordStart, cursorPos - wordStart);
            
            auto completions = getCompletions(partial);
            
            if (completions.size() == 1) {
                std::string completion = completions[0];
                line = line.substr(0, wordStart) + completion + line.substr(cursorPos);
                cursorPos = wordStart + completion.size();
                clearLine(prompt.size(), line.size() + 10);
                refreshLine(prompt, line, cursorPos);
            } else if (completions.size() > 1) {
                std::string common = getCommonPrefix(completions);
                if (common.size() > partial.size()) {
                    line = line.substr(0, wordStart) + common + line.substr(cursorPos);
                    cursorPos = wordStart + common.size();
                    clearLine(prompt.size(), line.size() + 10);
                    refreshLine(prompt, line, cursorPos);
                } else {
                    std::cout << '\n';
                    for (const auto& c : completions) {
                        std::cout << c << "  ";
                    }
                    std::cout << '\n';
                    refreshLine(prompt, line, cursorPos);
                }
            }
            continue;
        }
        
        if (vk == VK_BACK) {
            if (cursorPos > 0) {
                line.erase(cursorPos - 1, 1);
                cursorPos--;
                clearLine(prompt.size(), line.size() + 1);
                refreshLine(prompt, line, cursorPos);
            }
            continue;
        }
        
        if (vk == VK_DELETE) {
            if (cursorPos < line.size()) {
                line.erase(cursorPos, 1);
                clearLine(prompt.size(), line.size() + 1);
                refreshLine(prompt, line, cursorPos);
            }
            continue;
        }
        
        if (vk == VK_LEFT) {
            if (cursorPos > 0) {
                cursorPos--;
                std::cout << '\b';
                std::cout.flush();
            }
            continue;
        }
        
        if (vk == VK_RIGHT) {
            if (cursorPos < line.size()) {
                std::cout << line[cursorPos];
                std::cout.flush();
                cursorPos++;
            }
            continue;
        }
        
        if (vk == VK_UP) {
            if (!m_history.empty() && historyNav > 0) {
                if (historyNav == m_history.size()) {
                    savedLine = line;
                }
                historyNav--;
                size_t oldLen = line.size();
                line = m_history[historyNav];
                cursorPos = line.size();
                clearLine(prompt.size(), oldLen > line.size() ? oldLen : line.size());
                refreshLine(prompt, line, cursorPos);
            }
            continue;
        }
        
        if (vk == VK_DOWN) {
            if (historyNav < m_history.size()) {
                size_t oldLen = line.size();
                historyNav++;
                if (historyNav == m_history.size()) {
                    line = savedLine;
                } else {
                    line = m_history[historyNav];
                }
                cursorPos = line.size();
                clearLine(prompt.size(), oldLen > line.size() ? oldLen : line.size());
                refreshLine(prompt, line, cursorPos);
            }
            continue;
        }
        
        if (vk == VK_HOME) {
            while (cursorPos > 0) {
                std::cout << '\b';
                cursorPos--;
            }
            std::cout.flush();
            continue;
        }
        
        if (vk == VK_END) {
            while (cursorPos < line.size()) {
                std::cout << line[cursorPos];
                cursorPos++;
            }
            std::cout.flush();
            continue;
        }
        
        if (ch >= 32 && ch < 127) {
            line.insert(cursorPos, 1, ch);
            cursorPos++;
            if (cursorPos == line.size()) {
                std::cout << ch;
                std::cout.flush();
            } else {
                refreshLine(prompt, line, cursorPos);
            }
        }
    }
    
    SetConsoleMode(m_hInput, originalMode);
    
    if (!line.empty()) {
        if (m_history.empty() || m_history.back() != line) {
            m_history.push_back(line);
        }
    }
    
    return line;
}

}