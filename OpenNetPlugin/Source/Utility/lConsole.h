/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-23
    Notes:
        A console that works across plugins.
*/

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

class lConsole
{
    // Output streams.
    void *FileHandle;
    void *StreamHandle;

    // Scrollback.
    bool ShouldPrintScrollback;
    uint64_t StartupTimestamp;
    uint64_t LastTimestamp;
    uint32_t ScrollbackLineCount;
    struct lLine *ScrollbackLines;

    // Progress bars in the scrollback.
    uint32_t ProgressbarCount;
    struct lProgressBar *Progressbars;

public:
    // Initialization of the console.
    bool Initialize(void *FileHandle = nullptr, void *Stream = stdout, uint32_t ScreenbufferSize = 30);
    bool Initialize(const char *Filename = nullptr, void *Stream = stdout, uint32_t ScreenbufferSize = 30);

    // Console modification.
    void ChangeOutputstream(void *NewStream);
    void ChangeOutputfile(void *NewFilehandle, bool CloseCurrentHandle = true);
    void ChangeWindowname(const char *NewName);
    void ChangeWindowsize(uint16_t Width, uint16_t Height);
    void ToggleScrollbackPrinting();

    // Enqueue data.
    void PrintStringRaw(struct lLine *String);                                  // > String
    void PrintStringRaw(const char *String);                                    // > String
    void PrintColoredString(const char *String, uint32_t RGBA);                 // > String
    void PrintTimedString(const char *String, bool AbsoluteTime = true);        // > Time | String
    uint32_t PrintProgress(uint32_t Start, uint32_t End, const char *String);   // > [%] | String

    // Progress management.
    void UpdateProgress(uint32_t Id, uint32_t NewValue);
    void EndProgress(uint32_t Id);

    // Info for other plugins.
    void *ShareOutputfile();
    void *ShareOutputstream();

    // Internal thread for updating the console.
    void Int_UpdateThread();

    // Methods not to call from usercode.
    lConsole();
    ~lConsole();
};
