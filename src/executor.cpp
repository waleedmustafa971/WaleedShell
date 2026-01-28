#include "executor.hpp"
#include "shell.hpp"

namespace WaleedShell {

bool Executor::isCmdBuiltin(const std::string& program) {
    static std::vector<std::string> cmdBuiltins = {
        "dir", "echo", "type", "copy", "move", "del", "ren", "mkdir", "rmdir",
        "md", "rd", "set", "ver", "vol", "date", "time", "path", "title"
    };
    std::string lower = program;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return std::find(cmdBuiltins.begin(), cmdBuiltins.end(), lower) != cmdBuiltins.end();
}

std::string Executor::findExecutable(const std::string& program) {
    if (program.find('\\') != std::string::npos || 
        program.find('/') != std::string::npos ||
        program.find(':') != std::string::npos) {
        return program;
    }
    
    std::vector<std::string> extensions = {"", ".exe", ".cmd", ".bat", ".com"};
    
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
    
    for (const auto& dir : paths) {
        for (const auto& ext : extensions) {
            std::string fullPath = dir + "\\" + program + ext;
            if (GetFileAttributesA(fullPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                return fullPath;
            }
        }
    }
    
    return program;
}

std::string Executor::buildCommandLine(Command& cmd) {
    std::string cmdLine = cmd.program;
    
    for (const auto& arg : cmd.args) {
        cmdLine += " ";
        if (arg.find(' ') != std::string::npos) {
            cmdLine += "\"" + arg + "\"";
        } else {
            cmdLine += arg;
        }
    }
    
    return cmdLine;
}

int Executor::executeSingle(Command& cmd) {
    std::string cmdLine;
    
    if (isCmdBuiltin(cmd.program)) {
        cmdLine = "cmd.exe /c " + buildCommandLine(cmd);
    } else {
        cmdLine = buildCommandLine(cmd);
    }
    
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    
    HANDLE hInputRead = NULL;
    HANDLE hOutputWrite = NULL;
    
    if (cmd.inputRedirect.type == RedirectType::Input) {
        hInputRead = CreateFileA(
            cmd.inputRedirect.filename.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            &sa,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hInputRead == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Cannot open input file '" << cmd.inputRedirect.filename << "'\n";
            return 1;
        }
    }
    
    if (cmd.outputRedirect.type == RedirectType::Output) {
        hOutputWrite = CreateFileA(
            cmd.outputRedirect.filename.c_str(),
            GENERIC_WRITE,
            0,
            &sa,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hOutputWrite == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Cannot create output file '" << cmd.outputRedirect.filename << "'\n";
            if (hInputRead) CloseHandle(hInputRead);
            return 1;
        }
    } else if (cmd.outputRedirect.type == RedirectType::Append) {
        hOutputWrite = CreateFileA(
            cmd.outputRedirect.filename.c_str(),
            FILE_APPEND_DATA,
            0,
            &sa,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hOutputWrite == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Cannot open output file '" << cmd.outputRedirect.filename << "'\n";
            if (hInputRead) CloseHandle(hInputRead);
            return 1;
        }
    }
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    if (hInputRead || hOutputWrite) {
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdInput = hInputRead ? hInputRead : GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = hOutputWrite ? hOutputWrite : GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    }
    
    BOOL success = CreateProcessA(
        NULL,
        const_cast<char*>(cmdLine.c_str()),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );
    
    if (!success) {
        std::cerr << "Error: Command not found or failed to execute: " << cmd.program << "\n";
        if (hInputRead) CloseHandle(hInputRead);
        if (hOutputWrite) CloseHandle(hOutputWrite);
        return 1;
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if (hInputRead) CloseHandle(hInputRead);
    if (hOutputWrite) CloseHandle(hOutputWrite);
    
    return static_cast<int>(exitCode);
}

int Executor::executePipeline(Pipeline& pipeline) {
    if (pipeline.commands.size() == 1) {
        return executeSingle(pipeline.commands[0]);
    }
    
    size_t numCmds = pipeline.commands.size();
    std::vector<HANDLE> processes;
    HANDLE hPrevReadPipe = NULL;
    
    Command& firstCmd = pipeline.commands[0];
    bool firstIsShellBuiltin = m_shell && m_shell->isBuiltin(firstCmd.program);
    
    if (firstIsShellBuiltin) {
        std::string output = m_shell->executeBuiltinCapture(firstCmd);
        
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        
        HANDLE hReadPipe, hWritePipe;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
            std::cerr << "Error: Failed to create pipe\n";
            return 1;
        }
        
        DWORD written;
        WriteFile(hWritePipe, output.c_str(), static_cast<DWORD>(output.size()), &written, NULL);
        CloseHandle(hWritePipe);
        
        hPrevReadPipe = hReadPipe;
    }
    
    size_t startIdx = firstIsShellBuiltin ? 1 : 0;
    
    for (size_t i = startIdx; i < numCmds; ++i) {
        Command& cmd = pipeline.commands[i];
        
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        
        HANDLE hReadPipe = NULL;
        HANDLE hWritePipe = NULL;
        
        if (i < numCmds - 1) {
            if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
                std::cerr << "Error: Failed to create pipe\n";
                return 1;
            }
        }
        
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        ZeroMemory(&pi, sizeof(pi));
        
        if (i == startIdx && !firstIsShellBuiltin) {
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        } else {
            si.hStdInput = hPrevReadPipe;
        }
        
        if (i == numCmds - 1) {
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        } else {
            si.hStdOutput = hWritePipe;
        }
        
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        
        std::string cmdLine;
        if (isCmdBuiltin(cmd.program)) {
            cmdLine = "cmd.exe /c " + buildCommandLine(cmd);
        } else {
            cmdLine = buildCommandLine(cmd);
        }
        
        BOOL success = CreateProcessA(
            NULL,
            const_cast<char*>(cmdLine.c_str()),
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &si,
            &pi
        );
        
        if (hPrevReadPipe) {
            CloseHandle(hPrevReadPipe);
            hPrevReadPipe = NULL;
        }
        
        if (hWritePipe) {
            CloseHandle(hWritePipe);
            hWritePipe = NULL;
        }
        
        if (!success) {
            std::cerr << "Error: Failed to execute: " << cmd.program << "\n";
            if (hReadPipe) CloseHandle(hReadPipe);
            for (auto h : processes) CloseHandle(h);
            return 1;
        }
        
        processes.push_back(pi.hProcess);
        CloseHandle(pi.hThread);
        
        hPrevReadPipe = hReadPipe;
    }
    
    WaitForMultipleObjects(static_cast<DWORD>(processes.size()), processes.data(), TRUE, INFINITE);
    
    DWORD exitCode = 0;
    GetExitCodeProcess(processes.back(), &exitCode);
    
    for (auto h : processes) {
        CloseHandle(h);
    }
    
    return static_cast<int>(exitCode);
}

int Executor::execute(Pipeline& pipeline) {
    if (!pipeline.isValid || pipeline.commands.empty()) {
        return 1;
    }
    return executePipeline(pipeline);
}

}