/*
    Initial author: (https://github.com/)Convery
    Started: 2015-08-16
    License: LGPL 3.0
    Notes:
        General defines used in the project.
*/

#pragma once

// Disable warnings.
#define _CRT_SECURE_NO_WARNINGS

// Debug information.
#define DEBUG_NET       // Log information about the network.
#define DEBUG_FS        // Log information about the filesystem.
#define DEBUG_PRF       // Log information about performance.
#define DEBUG_MEM       // Log information about memory usage.

// Debug features.
#ifdef _DEBUG
#define SafeString(string) string
#define DebugPrint(string) Global::PluginConsole.PrintStringRaw(string);
#define PrintFunction() DebugPrint(__func__)
#define HardDebugBreak() MessageBoxA(0, __FUNCTION__, 0, 0)
#else
#define SafeString(string) ""
#define DebugPrint(string)
#define PrintFunction()
#define HardDebugBreak()
#endif

// Type specific debug info.
#ifdef DEBUG_NET
#define nDebugPrint(string, ...) DebugPrint(va(string, ##__VA_ARGS__))
#else
#define nDebugPrint(string, ...)
#endif
#ifdef DEBUG_FS
#define fDebugPrint(string, ...) DebugPrint(va(string, ##__VA_ARGS__))
#else
#define fDebugPrint(string, ...)
#endif
#ifdef DEBUG_PRF
#define pDebugPrint(string, ...) DebugPrint(va(string, ##__VA_ARGS__))
#else
#define pDebugPrint(string, ...)
#endif
#ifdef DEBUG_MEM
#define mDebugPrint(string, ...) DebugPrint(va(string, ##__VA_ARGS__))
#else
#define mDebugPrint(string, ...)
#endif
