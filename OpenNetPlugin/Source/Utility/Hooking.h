/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-16
    Notes:
        Reduced this file to only include what's used.
*/

#pragma once
#include "..\STDInclude.h"
#include <WinSock2.h>

// Prefered getproc address function as it gives more info.
inline void *PE_DebugGetProcAddress(const char *Modulename, const char *Functionname)
{
    /*
        Inspired by fancycodes MemoryGetProcAddress.
        See NetworkModules/MemoryModule.h for copyright.
    */

    void *ModuleBase = nullptr;
    PIMAGE_NT_HEADERS NTHeader;
    PIMAGE_EXPORT_DIRECTORY Exports;

    // Find a module by name in the proccess.
    ModuleBase = GetModuleHandleA(Modulename);
    if (!ModuleBase)
    {
        fDebugPrint("%s: Module \"%s\" could not be found in the process.", __func__, Modulename);
        return nullptr;
    }

    // Read the PE headers.
    PIMAGE_DOS_HEADER DOSHeader;
    DOSHeader = (PIMAGE_DOS_HEADER)ModuleBase;
    if (DOSHeader->e_magic == IMAGE_DOS_SIGNATURE)
    {
        NTHeader = (PIMAGE_NT_HEADERS)(DOSHeader->e_lfanew + (size_t)ModuleBase);
        if (NTHeader->Signature != IMAGE_NT_SIGNATURE)
        {
            fDebugPrint("%s: Module \"%s\" is not a valid win32 module.", __func__, Modulename);
            return nullptr;
        }
    }
    else
    {
        fDebugPrint("%s: Module \"%s\" is not a valid win32 module.", __func__, Modulename);
        return nullptr;
    }

    // Verify that the module has an export table.
    if (NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
    {
        fDebugPrint("%s: Module \"%s\" does not have a export table.", __func__, Modulename);
        return nullptr;
    }

    // Get the directory and verify that the module exports functions.
    Exports = (PIMAGE_EXPORT_DIRECTORY)((NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) + (size_t)ModuleBase);
    if (Exports->NumberOfNames == 0 || Exports->NumberOfFunctions == 0)
    {
        fDebugPrint("%s: Module \"%s\" does not have any exports.", __func__, Modulename);
        return nullptr;
    }

    // Find a function by index unless a string is specified.
    if (HIWORD(Functionname) == 0)
    {
        // Return the address we want.
        return (void *)((size_t)ModuleBase + (*(uint32_t *)((size_t)ModuleBase + Exports->AddressOfFunctions + ((LOWORD(Functionname) - Exports->Base) * sizeof(uint32_t)))));
    }

    // Iterate through the named functions until we find ours.
    uint32_t *RelativeNamePointer = (uint32_t *)(Exports->AddressOfNames + (size_t)ModuleBase);
    uint16_t *RelativeOrdinalPointer = (uint16_t *)(Exports->AddressOfNameOrdinals + (size_t)ModuleBase);

    for (uint32_t i = 0; i < Exports->NumberOfNames; ++i)
    {
        if (!_stricmp((const char *)((*RelativeNamePointer) + (size_t)ModuleBase), Functionname))
        {
            // Return the address we want.
            return (void *)((size_t)ModuleBase + (*(uint32_t *)((size_t)ModuleBase + Exports->AddressOfFunctions + (*RelativeOrdinalPointer * sizeof(uint32_t)))));
        }
        else
        {
            RelativeNamePointer++;
            RelativeOrdinalPointer++;
        }
    }

    // This block is reached if no function was found.
    fDebugPrint("%s: Could not find the specified export in module \"%s\".", __func__, Modulename);
    return nullptr;
}
inline void *DebugGetProcAddress(const char *Modulename, const char *Functionname)
{
#ifdef _WIN32
    return PE_DebugGetProcAddress(Modulename, Functionname);
#else

#endif
}

// Import Address Table hook function.
inline void *PE_ReplaceIATEntry(const char *Modulename, const char *Functionname, void *NewAddress, uint64_t Imagebase)
{
    void *OldAddress;
    void *TargetAddress;
    PIMAGE_NT_HEADERS NTHeader;
    PIMAGE_IMPORT_DESCRIPTOR Imports;
    PIMAGE_THUNK_DATA ImportThunkData;

    // Find the export in the module we are interested in.
    TargetAddress = GetProcAddress(GetModuleHandleA(Modulename), Functionname);
    // TargetAddress = PE_DebugGetProcAddress(Modulename, Functionname);
    if (TargetAddress == nullptr)
    {
        // There should be plenty of debug info from GetExportOrdinal.
        return nullptr;
    }

    // Verify the imagebase, replace with the main module.
    if (Imagebase == 0)
        Imagebase = (uint64_t)GetModuleHandleA(NULL);

    // Read the PE headers.
    PIMAGE_DOS_HEADER DOSHeader;
    DOSHeader = (PIMAGE_DOS_HEADER)Imagebase;
    if (DOSHeader->e_magic == IMAGE_DOS_SIGNATURE)
    {
        NTHeader = (PIMAGE_NT_HEADERS)(DOSHeader->e_lfanew + (size_t)Imagebase);
        if (NTHeader->Signature != IMAGE_NT_SIGNATURE)
        {
            // This should never, ever; be called. But it's in here for clarity.
            fDebugPrint("%s: The current application is not a win32 program.", __func__);
            return nullptr;
        }
    }
    else
    {
        // This should never, ever; be called. But it's in here for clarity.
        fDebugPrint("%s: The current application is not a win32 program.", __func__);
        return nullptr;
    }

    // Verify that the application has an import table.
    if (NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
    {
        fDebugPrint("%s: The current application does not have an import table.", __func__);
        return nullptr;
    }

    // Get the directory and start iterating through the list.
    Imports = (PIMAGE_IMPORT_DESCRIPTOR)((NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) + (size_t)Imagebase);
    for (uint32_t i = 0; Imports[i].Characteristics != 0; i++)
    {
        // Find the target module in the table.
        if (!_stricmp((char *)(Imports[i].Name + (size_t)Imagebase), Modulename))
        {
            // Iterate through the thunks.
            for (uint32_t c = 0;; ++c)
            {
                // Get the next thunk, check if it's the last.
                ImportThunkData = (PIMAGE_THUNK_DATA)((size_t)Imports[i].OriginalFirstThunk + c * sizeof(IMAGE_THUNK_DATA) + (size_t)Imagebase);
                if (ImportThunkData->u1.AddressOfData == NULL)
                    break;

                // Imported by ordinal.
                if (ImportThunkData->u1.Ordinal & IMAGE_ORDINAL_FLAG)
                {
                    PIMAGE_THUNK_DATA OrdinalThunk = (PIMAGE_THUNK_DATA)((size_t)Imports[i].FirstThunk + c * sizeof(IMAGE_THUNK_DATA) + (size_t)Imagebase);

                    if (TargetAddress == (void *)OrdinalThunk->u1.Function)
                    {
                        OldAddress = (void *)OrdinalThunk->u1.Function;
#ifdef _WIN64
                        OrdinalThunk->u1.Function = (unsigned long long)NewAddress;
#else
                        OrdinalThunk->u1.Function = (unsigned long)NewAddress;
#endif
                        return OldAddress;
                    }
                    else
                    {
                        continue;
                    }
                }

                // Imported by name.
                if (!_stricmp(((PIMAGE_IMPORT_BY_NAME)(ImportThunkData->u1.AddressOfData + (size_t)Imagebase))->Name, Functionname))
                {
                    PIMAGE_THUNK_DATA NameThunk = (PIMAGE_THUNK_DATA)((size_t)Imports[i].FirstThunk + c * sizeof(IMAGE_THUNK_DATA) + (size_t)Imagebase);
                    OldAddress = (void *)NameThunk->u1.Function;
#ifdef _WIN64
                    NameThunk->u1.Function = (unsigned long long)NewAddress;
#else
                    NameThunk->u1.Function = (unsigned long)NewAddress;
#endif
                    return OldAddress;
                }
            }
        }
    }

    fDebugPrint("%s: Could not find the export \"%s::%s\"", __func__, Modulename, Functionname);
    return nullptr;
}
inline void *ReplaceIATEntry(const char *Modulename, const char *Functionname, void *NewAddress, uint64_t Imagebase = 0)
{
#ifdef _WIN32
    return PE_ReplaceIATEntry(Modulename, Functionname, NewAddress, Imagebase);
#else

#endif
}
