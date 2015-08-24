/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-24
    Notes:
        Load servers from a CSV file.
        #host, servermodule, license
*/

#include "ModuleManager.h"
#include "MemoryModule.h"
#include "..\Interfaces\IServer.h"

// OpennetModule.
struct onModule
{
    std::string Filename;
    std::basic_string<uint8_t> DecryptedFile;
    HMEMORYMODULE ModuleHandle;
};

// Class properties.
std::vector<onModule *> ModuleManager::ModuleList;
CSVManager              *ModuleManager::FileNanager;

// Decrypt a module using the license key.
bool ModuleManager::DecryptModule(const char *Filename, const char *License)
{
    uint8_t EncryptionKey[24]{};
    uint8_t InitialVector[24]{};
    uint64_t HashStorage{};
    uint32_t Modulecount{ ModuleList.size() };
    onModule *NewModule;
    FILE *OnDiskModule;

    // Read the file from disk.
    if (fopen_s(&OnDiskModule, Filename, "rb"))
    {
        fDebugPrint("Failed to read a onModule from disk.", "");
        return false;
    }
    else
    {
        uint32_t Filesize{};
        uint8_t *Filebuffer;

        // Get the filesize.
        fseek(OnDiskModule, 0, SEEK_END);
        Filesize = ftell(OnDiskModule);
        fseek(OnDiskModule, 0, SEEK_SET);

        // Create a filebuffer and read.
        Filebuffer = new uint8_t[Filesize + 1]();
        if (fread_s(Filebuffer, Filesize + 1, 1, Filesize, OnDiskModule) == Filesize)
        {
            // If no license is supplied, we assume it's not encrypted.
            if (License != nullptr)
            {
                // Generate an IV for the decryption.
                HashStorage = FNV1_64Hash((void *)License, strlen(License));
                memcpy(InitialVector + 0, &HashStorage, 8);
                HashStorage = FNV1_64Hash(&HashStorage + 0, 4);
                memcpy(InitialVector + 8, &HashStorage, 8);
                HashStorage = FNV1_64Hash(&HashStorage + 0, 4);
                memcpy(InitialVector + 16, &HashStorage, 8);

                // Generate a key for the decryption.
                HashStorage = FNV1_64Hash((void *)License, strlen(License));
                memcpy(InitialVector + 0, &HashStorage, 8);
                HashStorage = FNV1_64Hash(&HashStorage + 4, 4);
                memcpy(InitialVector + 8, &HashStorage, 8);
                HashStorage = FNV1_64Hash(&HashStorage + 4, 4);
                memcpy(InitialVector + 16, &HashStorage, 8);

                // TODO, decrypt.
            }

            // Add our new module to the vector.
            NewModule = new onModule();
            NewModule->Filename = Filename;
            NewModule->DecryptedFile.append(Filebuffer, Filesize);
            ModuleList.push_back(NewModule);
        }

        // Deallocate the buffer.
        SecureZeroMemory(Filebuffer, Filesize + 1);
        delete[] Filebuffer;
    }

    return ModuleList.size() > Modulecount;
};

// Load the module into memory.
bool ModuleManager::LoadModule(onModule *Module)
{
    // Load the module from memory.
    Module->ModuleHandle = MemoryLoadLibrary(Module->DecryptedFile.data());
    return Module->ModuleHandle != NULL;
};

// Create a server with the specified host.
IServer *ModuleManager::CreateServerInstance(onModule *Module, const char *Hostname)
{
    // This should not happen.
    if (Module->ModuleHandle == NULL)
    {
        fDebugPrint("%s: Tried to call an invalid handle.", __func__);
        return nullptr;
    }

    IServer *(*CreateServer)(const char *) = (IServer *(*)(const char *))MemoryGetProcAddress(Module->ModuleHandle, "CreateServer");

    // The developer of the modue forgot to export the function.
    if (!CreateServer)
    {
        fDebugPrint("%s: Modue did not export \"CreateServer\"", __func__);
        return nullptr;
    }

    return CreateServer(Hostname);
};
