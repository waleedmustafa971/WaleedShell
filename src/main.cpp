#include "common.hpp"
#include "shell.hpp"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    WaleedShell::Shell shell;
    shell.run();
    
    return 0;
}