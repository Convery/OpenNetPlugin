/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-16
    Notes:
        Creates a variadic string.
*/

#pragma once
#include "..\STDInclude.h"
#include <mutex>
#include <stdarg.h>

static char VariadicBuffer[4][2048];
static uint32_t NextVariadicBufferIndex;
static std::mutex vaThreadSafe;

inline const char *va(const char *fmt, ...)
{
    va_list VarArgs;
    int32_t StringLength = 0;
    char *DestinationBuffer = nullptr;

    vaThreadSafe.lock();
    DestinationBuffer = &VariadicBuffer[NextVariadicBufferIndex][0];
    memset(DestinationBuffer, 0, 2048);							// Clear any old data..
    NextVariadicBufferIndex = (NextVariadicBufferIndex + 1) % 4;		// Mod by buffercount.

    va_start(VarArgs, fmt);
    StringLength = _vsnprintf_s(DestinationBuffer, 2048, _TRUNCATE, fmt, VarArgs);
    va_end(VarArgs);

    vaThreadSafe.unlock();
    if (StringLength < 0 || StringLength == 2048)
    {
        fDebugPrint("%s - Attempted to overrun string, increase the buffer. StrLen: %i", __func__, StringLength);
    }

    return DestinationBuffer;
}