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

inline PIMAGE_NT_HEADERS xImageNTHeader(uint64_t ImgBase)
{
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS Image;

    DosHeader = (PIMAGE_DOS_HEADER)ImgBase;

    if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE)
    {
        Image = reinterpret_cast<PIMAGE_NT_HEADERS> (DosHeader->e_lfanew + ImgBase);
        if (Image->Signature == IMAGE_NT_SIGNATURE)
        {
            return (PIMAGE_NT_HEADERS)(DosHeader->e_lfanew + ImgBase);
        }
    }
    return 0;
}
inline size_t GetIATAddress(const char* ModuleName, const char* FunctionName, uint64_t ImgBase)
{
    PIMAGE_NT_HEADERS Image;
    PIMAGE_IMPORT_DESCRIPTOR ImportDesc;
    PIMAGE_THUNK_DATA ThunkData;

    Image = xImageNTHeader(ImgBase);
    if (!Image)
        return 0;

    ImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((Image->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) + (size_t)ImgBase);
    for (int i = 0;; i++)
    {
        if (ImportDesc->Name == NULL)
            break;

        if (!_stricmp((char *)(ImportDesc->Name + ImgBase), ModuleName))
        {
            if ((ImportDesc->OriginalFirstThunk) != NULL)
            {
                for (int c = 0;; c++)
                {
                    ThunkData = (PIMAGE_THUNK_DATA)((size_t)ImportDesc->OriginalFirstThunk + c * sizeof(IMAGE_THUNK_DATA) + ImgBase);

                    if (ThunkData->u1.AddressOfData == NULL)
                        break;

                    if (!_stricmp((char *)(((PIMAGE_IMPORT_BY_NAME)((size_t)ThunkData->u1.AddressOfData + ImgBase))->Name), FunctionName))
                    {
                        return (size_t)(((PIMAGE_THUNK_DATA)((size_t)ImportDesc->FirstThunk + c * sizeof(IMAGE_THUNK_DATA) + (size_t)ImgBase))->u1.Function);
                    }
                }
            }
        }
    }

    return 0;
}
inline size_t WriteIATAddress(const char* ModuleName, const char* FunctionName, uint64_t ImgBase, void *NewProc)
{
    PIMAGE_NT_HEADERS Image;
    PIMAGE_IMPORT_DESCRIPTOR ImportDesc;
    PIMAGE_THUNK_DATA ThunkData;
    size_t oldValue;

    Image = xImageNTHeader(ImgBase);

    if (!Image) return 0;

    ImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((Image->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) + (size_t)ImgBase);

    HMODULE origHandle = nullptr;
    LPVOID origProc = NULL;

    if (ModuleName != NULL)
        origHandle = GetModuleHandle(ModuleName);
    if (origHandle != nullptr)
        origProc = GetProcAddress(origHandle, FunctionName);
    else
        return 0;

    for (int i = 0;; i++)
    {
        if (ImportDesc->Name == NULL) break;

        if (!_stricmp((char*)((size_t)ImportDesc->Name + (size_t)ImgBase), ModuleName))
        {
            if ((ImportDesc->OriginalFirstThunk) != 0)
            {
                for (int c = 0;; c++)
                {
                    ThunkData = (PIMAGE_THUNK_DATA)((size_t)ImportDesc->OriginalFirstThunk + c*sizeof(IMAGE_THUNK_DATA) + (size_t)ImgBase);

                    if (ThunkData->u1.AddressOfData == NULL) break;

                    size_t ordinalNumber = ThunkData->u1.AddressOfData & 0xFFFFFFF;

                    if (GetProcAddress(origHandle, (LPCSTR)ordinalNumber) == origProc)
                    {
                        oldValue = (size_t)(((PIMAGE_THUNK_DATA)((size_t)ImportDesc->FirstThunk + c*sizeof(IMAGE_THUNK_DATA) + (size_t)ImgBase))->u1.Function);
                        ((PIMAGE_THUNK_DATA)((size_t)ImportDesc->FirstThunk + c*sizeof(IMAGE_THUNK_DATA) + (size_t)ImgBase))->u1.Function = (size_t)NewProc;
                        return oldValue;
                    }
                }
            }
        }
        ImportDesc++;
    }

    return 0;
}
