/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-18
    Notes:
        Plugin exports for use with various APIs.
*/

#include "STDInclude.h"
#include "Network\NTServerManager.h"

// Version information.
#define USING_REDACTED_API_2
#define USING_REDACTED_API_3

// Global variables.
std::vector<const char *> Global::Dependencies = {};
std::unordered_map<const char */*Key*/, const char */*Value*/> Global::EnvKeyVal;
std::unordered_map < const char */*Statement*/, void */*Callback*/> Global::ComCallbacks;
lConsole Global::PluginConsole;

extern "C"
{
    // 2014 Redacted API.
#ifdef USING_REDACTED_API_2
    __declspec(dllexport) int32_t __cdecl PreInit()
    {
        // Initialize the console so we can log errors.
        Global::PluginConsole.Initialize(SafeString("Openet.log"));

        // Initialize the platform logic.
#ifdef _WIN32
        NTServerManager::InitializeImportHooks();
#else

#endif

        return TRUE;
    };
    __declspec(dllexport) int32_t __cdecl PostInit()
    {
        return TRUE;
    };
    __declspec(dllexport) int32_t __cdecl AuthorInfo()
    {
#ifdef _WIN64
        return 0;
#else
        return (int32_t)Global::AuthorSite;
#endif
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
        // Initialize the console so we can log errors.
        Global::PluginConsole.Initialize(SafeString("Openet.log"));

        // Initialize the platform logic.
#ifdef _WIN32
        NTServerManager::InitializeImportHooks();
#else

#endif

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
        return Global::PluginName;
    };
    __declspec(dllexport) const char *__stdcall Info_AuthorName(void)
    {
        return Global::AuthorName;
    };
    __declspec(dllexport) const char *__stdcall Info_AuthorSite(void)
    {
        return Global::AuthorSite;
    };

    // Environmental settings.
    __declspec(dllexport) uint32_t __stdcall Env_DependencyCount(void)
    {
        return (uint32_t)min(UINT32_MAX, Global::Dependencies.size());
    };
    __declspec(dllexport) const char *__stdcall Env_DependencyNameByIndex(int32_t Index)
    {
        if (Index >= (int32_t)min(INT32_MAX, Global::Dependencies.size()) && Index >= 0)
            return "Out of bounds";
        else
            return Global::Dependencies[Index];
    };
    __declspec(dllexport) const char *__stdcall Env_GetValue(const char *Key)
    {
        try
        {
            auto KeyIterator = Global::EnvKeyVal.find(Key);
            
            // Return based on the index.
            if (KeyIterator == Global::EnvKeyVal.end())
                return KeyIterator->second;
            else
                return "Key not found";
        }
        catch (...)
        {
            return "Threw an exception";
        }
    };
    __declspec(dllexport) void __stdcall Env_SetValue(const char *Key, const char *Value)
    {
        Global::EnvKeyVal[Key] = Value;
    };  
    __declspec(dllexport) void __stdcall Env_SetDebugOutputStream(void *StreamHandle)
    {
        Global::PluginConsole.ChangeOutputstream(StreamHandle);
    };

    // Communication between plugins.
    __declspec(dllexport) void __stdcall Com_ProcessMessage(const char *Caller, uint8_t *Message, uint32_t Length)
    {

    };
    __declspec(dllexport) void __stdcall Com_RegisterCallback(const char *Caller, char *String, void *Callback)
    {

    };
#endif
};
