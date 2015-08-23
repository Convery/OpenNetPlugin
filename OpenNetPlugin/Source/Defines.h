/*
    Initial author: (https://github.com/)Convery
    Started: 2015-08-16
    License: LGPL 3.0
    Notes:
        General defines used in the project.
*/

#pragma once

// Debug information.
#define DEBUG_NET       // Log information about the network.
#define DEBUG_FS        // Log information about the filesystem.
#define DEBUG_PRF       // Log information about performance.
#define DEBUG_MEM       // Log information about memory usage.

// Debug features.
#ifdef _DEBUG
#define SafeString(string) string
#define DebugPrint(type, string) Global::PluginConsole.PrintStringRaw(string);
#define PrintFunction() DebugPrint(3, __func__)
#define HardDebugBreak() MessageBoxA(0, __FUNCTION__, 0, 0)
#else
#define SafeString(string) ""
#define DebugPrint(type, string)
#define PrintFunction()
#define HardDebugBreak()
#endif

// Type specific debug info.
#define nDebugPrint(string, ...) DebugPrint(1, va(string, ##__VA_ARGS__))
#define fDebugPrint(string, ...) DebugPrint(2, va(string, ##__VA_ARGS__))
#define pDebugPrint(string, ...) DebugPrint(3, va(string, ##__VA_ARGS__))
#define mDebugPrint(string, ...) DebugPrint(4, va(string, ##__VA_ARGS__))


