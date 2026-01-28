#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <cstring>

namespace WaleedShell {
    constexpr const char* SHELL_NAME = "WaleedShell";
    constexpr const char* VERSION = "1.0.0";
    constexpr const char* PROMPT = "WaleedShell> ";
}