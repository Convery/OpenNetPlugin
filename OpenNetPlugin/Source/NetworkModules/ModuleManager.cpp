/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-24
    Notes:
        Load servers from a CSV file.
        #host, servermodule, license
*/

#include "ModuleManager.h"
#include "..\Interfaces\IServer.h"

// OpennetModule.
struct onModule
{
    std::string Filename;
    std::basic_string<uint8_t> DecryptedFile;
    void *FileHandle;
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
bool ModuleManager::LoadModule(struct onModule *Module)
{
    return false;
};

// Create a server with the specified host.
IServer *ModuleManager::CreateServerInstance(struct onModule *Module, const char *Hostname)
{
    return nullptr;
};
