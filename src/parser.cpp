#include "parser.hpp"

namespace WaleedShell {

std::vector<std::string> Parser::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    char quoteChar = 0;
    
    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        
        if (inQuotes) {
            if (c == quoteChar) {
                inQuotes = false;
            } else {
                current += c;
            }
        } else {
            if (c == '"' || c == '\'') {
                inQuotes = true;
                quoteChar = c;
            } else if (c == ' ' || c == '\t') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else if (c == '|' || c == '<') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                tokens.push_back(std::string(1, c));
            } else if (c == '>') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                if (i + 1 < input.size() && input[i + 1] == '>') {
                    tokens.push_back(">>");
                    ++i;
                } else {
                    tokens.push_back(">");
                }
            } else {
                current += c;
            }
        }
    }
    
    if (!current.empty()) {
        tokens.push_back(current);
    }
    
    return tokens;
}

Command Parser::parseCommand(const std::vector<std::string>& tokens) {
    Command cmd;
    
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];
        
        if (token == "<") {
            if (i + 1 < tokens.size()) {
                cmd.inputRedirect.type = RedirectType::Input;
                cmd.inputRedirect.filename = tokens[++i];
            }
        } else if (token == ">") {
            if (i + 1 < tokens.size()) {
                cmd.outputRedirect.type = RedirectType::Output;
                cmd.outputRedirect.filename = tokens[++i];
            }
        } else if (token == ">>") {
            if (i + 1 < tokens.size()) {
                cmd.outputRedirect.type = RedirectType::Append;
                cmd.outputRedirect.filename = tokens[++i];
            }
        } else if (cmd.program.empty()) {
            cmd.program = token;
        } else {
            cmd.args.push_back(token);
        }
    }
    
    return cmd;
}

Pipeline Parser::parse(const std::string& input) {
    Pipeline pipeline;
    std::vector<std::string> tokens = tokenize(input);
    
    if (tokens.empty()) {
        pipeline.isValid = false;
        return pipeline;
    }
    
    std::vector<std::string> currentTokens;
    
    for (const auto& token : tokens) {
        if (token == "|") {
            if (currentTokens.empty()) {
                pipeline.isValid = false;
                pipeline.error = "Syntax error: unexpected '|'";
                return pipeline;
            }
            pipeline.commands.push_back(parseCommand(currentTokens));
            currentTokens.clear();
        } else {
            currentTokens.push_back(token);
        }
    }
    
    if (currentTokens.empty()) {
        pipeline.isValid = false;
        pipeline.error = "Syntax error: expected command after '|'";
        return pipeline;
    }
    
    pipeline.commands.push_back(parseCommand(currentTokens));
    return pipeline;
}

}