#pragma once
#include "common.hpp"

namespace WaleedShell {

enum class RedirectType {
    None,
    Input,
    Output,
    Append
};

struct Redirect {
    RedirectType type = RedirectType::None;
    std::string filename;
};

struct Command {
    std::string program;
    std::vector<std::string> args;
    Redirect inputRedirect;
    Redirect outputRedirect;
};

struct Pipeline {
    std::vector<Command> commands;
    bool isValid = true;
    std::string error;
};

class Parser {
public:
    Pipeline parse(const std::string& input);
    
private:
    std::vector<std::string> tokenize(const std::string& input);
    Command parseCommand(const std::vector<std::string>& tokens);
};

}