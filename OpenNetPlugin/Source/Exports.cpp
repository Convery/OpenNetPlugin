/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-18
    Notes:
        Plugin exports for use with various APIs.
*/

#include "STDInclude.h"

// Version information.
#define USING_REDACTED_API_2
#define USING_REDACTED_API_3

extern "C"
{
    // 2014 Redacted API.
#ifdef USING_REDACTED_API_2
    __declspec(dllexport) int32_t __cdecl PreInit()
    {
        return TRUE;
    };
    __declspec(dllexport) int32_t __cdecl PostInit()
    {
        return TRUE;
    };
    __declspec(dllexport) int32_t __cdecl AuthorInfo()
    {
        return 0;
    };
    __declspec(dllexport) int32_t __cdecl ExtendedInfo()
    {
        return 0;
    };
    __declspec(dllexport) int32_t __cdecl OfficialMod()
    {
        return FALSE;
    };
    __declspec(dllexport) int32_t __cdecl DependencyCount()
    {
        return 0;
    };
    __declspec(dllexport) const char *__cdecl GetDependency(uint32_t Index)
    {
        return "None";
    };
    __declspec(dllexport) int32_t __cdecl SendMessageB(const char *Message)
    {
        return FALSE;
    }
#endif

    // 2015 Redacted API.
#ifdef USING_REDACTED_API_3
    // Plugin state.
    __declspec(dllexport) bool __stdcall Plugin_PreGameInitialization(void)
    {
        return false;
    };
    __declspec(dllexport) bool __stdcall Plugin_PostGameInitialization(void)
    {
        return false;
    };
    __declspec(dllexport) bool __stdcall Plugin_Shutdown(void)
    {
        return false;
    };

    // Printed information.
    __declspec(dllexport) const char *__stdcall Info_PluginName(void)
    {
        return "OpenNet";
    };
    __declspec(dllexport) const char *__stdcall Info_AuthorName(void)
    {
        return "Convery";
    };
    __declspec(dllexport) const char *__stdcall Info_AuthorSite(void)
    {
        return "https://github.com/Convery";
    };

    // Environmental settings.
    __declspec(dllexport) uint32_t __stdcall Env_DependencyCount(void)
    {
        // Opennet cares not for other plugins.
        return 0;
    };
    __declspec(dllexport) const char *__stdcall Env_DependencyNameByIndex(uint32_t Index)
    {
        switch (Index)
        {
        default:
            return "Invalid Dep";
        }
    };
    __declspec(dllexport) const char *__stdcall Env_GetValue(const char *Key)
    {
        return "";
    };
    __declspec(dllexport) void __stdcall Env_SetValue(const char *Key, const char *Value)
    {

    };  
    __declspec(dllexport) void __stdcall Env_SetDebugOutputStream(void *StreamHandle)
    {

    };

    // Communication between plugins.
    __declspec(dllexport) void __stdcall Com_ProcessMessage(char *Caller, uint8_t *Message, uint32_t Length)
    {

    };
    __declspec(dllexport) void __stdcall Com_RegisterCallback(char *Caller, char *String, void *Callback)
    {

    };
#endif
};
