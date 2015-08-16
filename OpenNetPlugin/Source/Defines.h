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
#define SafeString(x) x
#define DebugPrint(x) x
#define PrintFunction()
#define HardDebugBreak() MessageBoxA(0, __FUNCTION__, 0, 0)
#else
#define SafeString(x) ""
#define DebugPrint(x)
#define PrintFunction()
#define HardDebugBreak()
#endif


