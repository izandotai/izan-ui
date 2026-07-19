#include "ui/shell/win_console.hpp"

#include <cstdio>
#include <ios>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace izan::ui {

#ifdef _WIN32
bool try_attach_parent_console()
{
    if (!AttachConsole(ATTACH_PARENT_PROCESS))
        return false;

    FILE* stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
    freopen_s(&stream, "CONIN$", "r", stdin);
    std::ios::sync_with_stdio();
    return true;
}
#else
bool try_attach_parent_console()
{
    return false;
}
#endif

}
