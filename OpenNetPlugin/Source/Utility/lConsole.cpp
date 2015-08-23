/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-23
    Notes:
        A console that works across plugins.
*/

#define _CRT_SECURE_NO_WARNINGS

#include "lConsole.h"
#include <Windows.h>
#include <thread>

// Internal structs.
struct lLine
{
    uint32_t RGBA;
    char String[256];
};
struct lProgressBar
{
    char ProgressToken[10];
    lLine PrependString;
    uint32_t ProgressBarID;
    uint32_t StartValue;
    uint32_t CurrentValue;
    uint32_t EndValue;
    bool Interrupted;
};

// Helpers.
const char *GetProgress(uint32_t ProgressType, uint32_t CurrentProgress, uint32_t TotalProgress)
{
    static char InternalBuffer[12]{ "CakeIsLife|" };
    double Percentage = float(CurrentProgress) / float(TotalProgress);
    uint8_t Characters = (uint8_t)((Percentage * 100 + 5) / 10) + 1;

    // Clear the previous result.
    memset(InternalBuffer, 0, 10);

    // Create a string based on the type.
    switch (ProgressType)
    {
    case 0:
        for (uint8_t i = 0; i < Characters; i++)
            InternalBuffer[i] = '#';
        break;
    case 1:
        for (uint8_t i = 0; i < Characters; i++)
            InternalBuffer[i] = '*';
        break;
    case 2:
        for (uint8_t i = 0; i < Characters; i++)
            InternalBuffer[i] = '-';
        break;
    case 3:
        for (uint8_t i = 0; i < Characters; i++)
            InternalBuffer[i] = '>';
        break;

    case 4:
        for (uint8_t i = 0; i < Characters - 1; i++)
            InternalBuffer[i] = '-';
        InternalBuffer[Characters - 1] = '>';
        break;

    case 5:
        sprintf(InternalBuffer, "%u%s", uint32_t(Percentage * 100), "%%");
        break;
    }

    return InternalBuffer;
}
uint32_t CompatibleRGBA(uint32_t RGBA)
{
    uint32_t Result = 0;

#ifdef _WIN32
    if (((uint8_t *)&RGBA)[1])
        Result |= (1 << 0); // Foreground blue.
    if (((uint8_t *)&RGBA)[2])
        Result |= (1 << 1); // Foreground green.
    if (((uint8_t *)&RGBA)[3])
        Result |= (1 << 2); // Foreground red.
    if (((uint8_t *)&RGBA)[0] > 128)
        Result |= (1 << 3); // Foreground intensity.
#else
#endif

    return Result;
}

// Initialization of the console.
static std::thread UpdateThread;
bool lConsole::Initialize(void *FileHandle, void *Stream, uint32_t ScreenbufferSize)
{
    // Allocate a console if we don't have one.
    if (!strstr(GetCommandLineA(), "-nocon"))
    {
        AllocConsole();

        // Take ownership of it.
        AttachConsole(GetCurrentProcessId());

        // Set the standard streams to use the console.
        freopen("CONOUT$", "w", (FILE *)&Stream);

        // Start the update thread.
        if (UpdateThread.joinable())
        {
            UpdateThread = std::thread(&lConsole::Int_UpdateThread, this);
        }
    }

    // Fill our properties.
    this->FileHandle = FileHandle;
    this->StreamHandle = Stream;
    this->ShouldPrintScrollback = ScreenbufferSize != 0;
    this->StartupTimestamp = GetTickCount64();
    this->LastTimestamp = this->StartupTimestamp;
    this->ScrollbackLineCount = ScreenbufferSize;
    this->ScrollbackLines = new lLine[this->ScrollbackLineCount]();
    this->ProgressbarCount = 0;

    return true;
}
bool lConsole::Initialize(const char *Filename, void *Stream, uint32_t ScreenbufferSize)
{
    remove(Filename);
    return Initialize(fopen(Filename, "a"), Stream, ScreenbufferSize);
}

// Console modification.
void lConsole::ChangeOutputstream(void *NewStream)
{
    this->StreamHandle = NewStream;
}
void lConsole::ChangeOutputfile(void *NewFilehandle, bool CloseCurrentHandle)
{
    if (CloseCurrentHandle)
        fclose((FILE *)&this->FileHandle);
    this->FileHandle = NewFilehandle;
}
void lConsole::ChangeWindowname(const char *NewName)
{
    SetConsoleTitleA(NewName);
}
void lConsole::ChangeWindowsize(uint16_t Width, uint16_t Height)
{
    _COORD coord;
    coord.X = Width;
    coord.Y = Height;

    _SMALL_RECT Rect;
    Rect.Top = 0;
    Rect.Left = 0;
    Rect.Bottom = Height - 1;
    Rect.Right = Width - 1;

    HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);      // Get handle.
    SetConsoleScreenBufferSize(Handle, coord);            // Set buffer size.
    SetConsoleWindowInfo(Handle, TRUE, &Rect);            // Set window size.
}
void lConsole::ToggleScrollbackPrinting()
{
    this->ShouldPrintScrollback = !this->ShouldPrintScrollback;
}

// Enqueue data.
void lConsole::PrintStringRaw(lLine *String)
{
    static bool ArrayIsFull = false;
    static uint32_t LastIndex = 0;

    // Check if the array is full.
    if (!ArrayIsFull)
    {
        for (uint32_t i = 0; i < ScrollbackLineCount; i++)
        {
            if (ScrollbackLines[i].String[0] == '\0')
            {
                LastIndex = i;
                break;
            }
        }

        if (LastIndex == ScrollbackLineCount - 1)
            ArrayIsFull = true;
    }

    // If the array is full, make room for the new line.
    if (ArrayIsFull)
    {
        // Clear the last line.
        ScrollbackLines[0].String[0] = '\0';
        ScrollbackLines[0].RGBA = 0;

        // Foreach, swap the pointers.
        for (uint32_t i = 0; i < ScrollbackLineCount - 1; i++)
        {
            static lLine TempPointer;

            TempPointer = ScrollbackLines[i + 1];
            ScrollbackLines[i] = TempPointer;
        }
    }

    // Copy the new line into the array.
    ScrollbackLines[LastIndex].RGBA = String->RGBA;
    memcpy(ScrollbackLines[LastIndex].String, String->String, 256);

    // Print to the stream instantly.
    if(FileHandle != nullptr)
        fprintf((FILE *)&FileHandle, String->String);
}
void lConsole::PrintStringRaw(const char *String)
{
    lLine InternalLine;

    InternalLine.RGBA = 0xffffff00;
    memset(InternalLine.String, 0, 256);
    strcpy(InternalLine.String, String);

    PrintStringRaw(&InternalLine);
}
void lConsole::PrintColoredString(const char *String, uint32_t RGBA)
{
    lLine InternalLine;

    InternalLine.RGBA = RGBA;
    memset(InternalLine.String, 0, 256);
    strcpy(InternalLine.String, String);

    PrintStringRaw(&InternalLine);
}
void lConsole::PrintTimedString(const char *String, bool AbsoluteTime)
{
    lLine InternalLine;
    char InternalBuffer[256];

    // Format the time and string into the buffer.
    sprintf(InternalBuffer, "%llu | %s", AbsoluteTime ? GetTickCount64() - this->StartupTimestamp : GetTickCount64() - this->LastTimestamp, String);

    InternalLine.RGBA = 0xffffff00;
    memset(InternalLine.String, 0, 256);
    strcpy(InternalLine.String, InternalBuffer);

    PrintStringRaw(&InternalLine);
}
uint32_t lConsole::PrintProgress(uint32_t Start, uint32_t End, const char *String)
{
    static lProgressBar TempProgress;
    static uint32_t RandSeed = 0;
    srand(RandSeed++);

    // Initialize our bar.
    TempProgress.StartValue = Start;
    TempProgress.CurrentValue = Start;
    TempProgress.EndValue = End;
    TempProgress.Interrupted = false;
    TempProgress.ProgressBarID = rand();
    strcpy_s(TempProgress.ProgressToken, 10, "29v7jehf");
    strcpy_s(TempProgress.PrependString.String, 256, String);
    TempProgress.PrependString.RGBA = 0xffffff00;

    // Allocate the new buffer.
    static lProgressBar *StaticProgressBuffer = nullptr;
    if (StaticProgressBuffer != nullptr)
        delete[] StaticProgressBuffer;
    StaticProgressBuffer = new lProgressBar[ProgressbarCount + 1];

    // Copy the old buffer over.
    for (uint32_t i = 0; i < ProgressbarCount; i++)
        StaticProgressBuffer[i] = Progressbars[i];
    StaticProgressBuffer[ProgressbarCount++] = TempProgress;

    // Delete the old buffer and replace with the new one.
    delete[] Progressbars;
    Progressbars = StaticProgressBuffer;

    // Print the string.
    static char InternalBuffer[256];
    memset(InternalBuffer, 0, 256);
    sprintf_s(InternalBuffer, 256, "%s | %s", TempProgress.ProgressToken, TempProgress.PrependString.String);
    PrintStringRaw(InternalBuffer);

    return TempProgress.ProgressBarID;
}
// Progress management.
void lConsole::UpdateProgress(uint32_t Id, uint32_t NewValue)
{
    // Iterate through the bars until the Id is found.
    for (uint32_t i = 0; i < ProgressbarCount; i++)
    {
        if (Progressbars[i].ProgressBarID == Id)
        {
            // Set the value and if done, stop updating it.
            Progressbars[i].CurrentValue = NewValue;
            if (NewValue == Progressbars[i].EndValue)
                EndProgress(Id);
        }
    }
}
void lConsole::EndProgress(uint32_t Id)
{
    // Iterate through the bars until the Id is found.
    for (uint32_t i = 0; i < ProgressbarCount; i++)
    {
        if (Progressbars[i].ProgressBarID == Id)
        {
            // Check for interruption.
            Progressbars[i].Interrupted = Progressbars[i].CurrentValue != Progressbars[i].EndValue;

            // Clear the progress token so it wont be found.
            Progressbars[i].ProgressToken[0] = '\0';
        }
    }
}

// Info for other plugins.
void *lConsole::ShareOutputfile()
{
    return this->FileHandle;
}
void *lConsole::ShareOutputstream()
{
    return this->StreamHandle;
}

// Internal thread for updating the console.
void lConsole::Int_UpdateThread()
{
    static std::chrono::milliseconds UpdateDelay(100);
    while (true)
    {
        // Delay so we don't waste time updating.
        std::this_thread::sleep_for(UpdateDelay);

        // If we have a stream handle.
        if (StreamHandle == nullptr || !ShouldPrintScrollback)
            continue;

        // Clear the console.
#ifdef _WIN32
        // Aparently system("cls") is bad practice as it spawns a shell.
        COORD topLeft = { 0, 0 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO screen;
        DWORD written;

        GetConsoleScreenBufferInfo(console, &screen);
        FillConsoleOutputCharacterA(
            console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
            );
        FillConsoleOutputAttribute(
            console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
            screen.dwSize.X * screen.dwSize.Y, topLeft, &written
            );
        SetConsoleCursorPosition(console, topLeft);
#else
        // But nix can use ANSI escapecodes.
        // CSI[2J to clear the console.
        // CSI[H to reset the cursor.
        fprintf(stdout, "\x1B[2J\x1B[H");
#endif

        // Save the old color settings.
        static CONSOLE_SCREEN_BUFFER_INFO OldTextColor;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &OldTextColor);

        // For each line in the scrollback.
        for (uint32_t i = 0; i < ScrollbackLineCount; i++)
        {
            // Set the output strings color.
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), CompatibleRGBA(ScrollbackLines[i].RGBA));

            // Search the string for progress bars.
            for (uint32_t c = 0; c < ProgressbarCount; c++)
            {
                if (strstr(ScrollbackLines[i].String, Progressbars[c].ProgressToken))
                {
                    static char ProgressBuffer[257];
                    memset(ProgressBuffer, 0, 257);

                    // Create the buffer.
                    for (int32_t k = 0; k < (int32_t)min(INT32_MAX, strlen(ScrollbackLines[i].String)) - 1; k++)
                    {
                        // Copy all the characters up until the token.
                        if (ScrollbackLines[i].String[k + 0] == Progressbars[c].ProgressToken[0] &&
                            ScrollbackLines[i].String[k + 1] == Progressbars[c].ProgressToken[1])
                        {
                            ProgressBuffer[k] = ScrollbackLines[i].String[k];
                        }
                        else
                        {
                            // Copy the progress bar into the buffer.
                            // TODO: Use the bar type.
                            strcpy(&ProgressBuffer[c], GetProgress(0, Progressbars[c].CurrentValue, Progressbars[c].EndValue));
                            break;
                        }
                    }

                    // Print the formated buffer.
                    printf(ProgressBuffer);
                    printf("\n");
                    goto LABEL_PRINT_DONE;
                }
            }

            // Print the string since there's no progress bar.
            printf(ScrollbackLines[i].String);
            printf("\n");

        LABEL_PRINT_DONE:;
        }

        // Reset the color.
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), OldTextColor.wAttributes);
    }
}

// Methods not to call from usercode.
lConsole::lConsole()
{

}
lConsole::~lConsole()
{

}
