#pragma once
#ifdef _WIN32

#include "logger.hpp"
#define WIN32_LEAN_AND_MEAN      // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#undef max

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

void execute();

#ifndef NO_ENTRY_POINT

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int) {
    log_trace("Starting Application...");

#define DEBUG
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    execute();
    log_trace("Exitting Application...");
    return 0;
}

#ifndef RELEASE_BUILD

// for allocated console in release build.
int main(int argc, char** argv) {
    logger_init();
    log_trace("This is a debug build");
    WinMain(0, 0, 0, 0);
}

#endif // RELEASE_BUILD

#endif
#endif

// for msvc:
// run command: "cl tyrant/main.cpp /std:c++17 /Fe:build/main.exe /Fo:build/"
//
// @TODO: make a C++ counterpart for premake/cmake.
