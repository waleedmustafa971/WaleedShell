@echo off
if not exist bin mkdir bin
g++ -std=c++20 -Wall -Wextra -I include -I src src/*.cpp src/modules/*.cpp -o bin/wshell.exe -static -lpsapi -liphlpapi -lws2_32
if %errorlevel% equ 0 (
    echo Build successful: bin/wshell.exe
) else (
    echo Build failed
)