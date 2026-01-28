#pragma once
#include "common.hpp"

namespace WaleedShell {

class InputHandler {
public:
    InputHandler();
    std::string readLine(const std::string& prompt);
    std::vector<std::string>& getHistory() { return m_history; }
    
private:
    std::vector<std::string> m_history;
    size_t m_historyIndex;
    HANDLE m_hInput;
    HANDLE m_hOutput;
    
    void clearLine(size_t promptLen, size_t lineLen);
    void refreshLine(const std::string& prompt, const std::string& line, size_t cursorPos);
    
    std::vector<std::string> getCompletions(const std::string& partial);
    std::vector<std::string> getPathCompletions(const std::string& partial);
    std::vector<std::string> getExecutableCompletions(const std::string& partial);
    std::string getCommonPrefix(const std::vector<std::string>& strings);
};

}