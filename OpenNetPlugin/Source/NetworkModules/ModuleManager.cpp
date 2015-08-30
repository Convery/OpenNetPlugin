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
#include "..\Network\NTServerManager.h"

// OpennetModule.
struct onModule
{
    std::string Filename;
    std::basic_string<uint8_t> DecryptedFile;
    bool LoadedFromDisk;
    union 
    {
        HMEMORYMODULE Memory;
        void *Disk;
    } ModuleHandle;
    
};

// Class properties.
std::vector<onModule *> ModuleManager::ModuleList;

// Decrypt a module using the license key.
bool ModuleManager::DecryptModule(const char *Filename, const char *License)
{
    uint8_t EncryptionKey[24]{};
    uint8_t InitialVector[24]{};
    uint64_t HashStorage{};
    uint32_t Modulecount{ ModuleList.size() };
    onModule *NewModule;
    FILE *OnDiskModule;
    std::string Path;

    // Create the path.
    Path.append("Plugins\\OpennetStorage\\Modules\\");
    Path.append(Filename);

    // Load from disk instead of memory.
    if (strstr(GetCommandLineA(), "-MODULES_FROM_DISK"))
    {
        // Loadlibrary wants an extension.
        Path.append(".dll");

        // Add our new module to the vector.
        NewModule = new onModule();
        NewModule->Filename = Filename;
        NewModule->LoadedFromDisk = true;
        NewModule->ModuleHandle.Disk = LoadLibraryA(Path.c_str());

        if (NewModule->ModuleHandle.Disk)
            ModuleList.push_back(NewModule);
        else
        {
            fDebugPrint("Failed to read a onModule from disk with error: %i", GetLastError());
            delete NewModule;
        }
            

        return ModuleList.size() > Modulecount;
    }

    // Read the file from disk.
    if (fopen_s(&OnDiskModule, Path.c_str(), "rb"))
    {
        fDebugPrint("Failed to read a onModule from disk with error: %i", GetLastError());
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
            if (strlen(License) > 4)
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
            NewModule->LoadedFromDisk = false;
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
    // Verify that the module should be loaded from memory.
    if (Module->LoadedFromDisk)
        return true;

    // Load the module from memory.
    Module->ModuleHandle.Memory = MemoryLoadLibrary(Module->DecryptedFile.data());
    return Module->ModuleHandle.Memory != NULL;
};

// Create a server with the specified host.
IServer *ModuleManager::CreateServerInstance(onModule *Module, const char *Hostname)
{
    IServer *(*CreateServer)(const char *);

    // This should not happen, but checking incase it's modified in the future.
    if (Module->LoadedFromDisk ? Module->ModuleHandle.Disk == NULL : Module->ModuleHandle.Memory == NULL)
    {
        fDebugPrint("%s: Tried to call an invalid handle.", __func__);
        return nullptr;
    }

    // Load from memory or EAT.
    if(Module->LoadedFromDisk)
        CreateServer = (IServer *(*)(const char *))DebugGetProcAddress(Module->Filename.c_str(), "CreateServer");
    else
        CreateServer = (IServer *(*)(const char *))MemoryGetProcAddress(Module->ModuleHandle.Memory, "CreateServer");

    // The developer of the modue forgot to export the function.
    if (!CreateServer)
    {
        fDebugPrint("%s: Module did not export \"CreateServer\"", __func__);
        return nullptr;
    }

    // Let this string be free, the module can clean it up.
    static char *FloatingString;
    FloatingString = new char[strlen(Hostname) + 1]();
    memcpy(FloatingString, Hostname, strlen(Hostname));

    return CreateServer(FloatingString);
};

// Load the CSV, create servers for each entry
int32_t ModuleManager::LoadAllModules()
{
    int32_t ServerCount{};
    CSVManager FileManager;

    // Read the entries from the CSV.
    FileManager.ReadFile("Plugins\\OpennetStorage\\Modules.csv", 3);

    // Decrypt and load all modules.
    for (uint32_t i = 0; i < FileManager.Buffer->size(); ++i)
    {
        // Check for duplicates.
        for (uint32_t c = 0; c < ModuleList.size(); ++c)
        {
            if (!_stricmp(ModuleList[c]->Filename.c_str(), (*FileManager.Buffer)[i][1].c_str()))
            {
                goto LABEL_SKIP_LOADING;
            }
        }

        // Decrypt and load.
        if (DecryptModule((*FileManager.Buffer)[i][1].c_str(), (*FileManager.Buffer)[i][2].c_str()))
        {
            if (!LoadModule(ModuleList.back()))
                ModuleList.pop_back();
        }
        else
        {
            fDebugPrint("%s: Failed to load module \"%s\".", __func__, (*FileManager.Buffer)[i][1].c_str());
        }

    LABEL_SKIP_LOADING:;
    }

    // Load all requested servers.
    for (uint32_t i = 0; i < FileManager.Buffer->size(); ++i)
    {
        for (uint32_t c = 0; c < ModuleList.size(); ++c)
        {
            if (!_stricmp(ModuleList[c]->Filename.c_str(), (*FileManager.Buffer)[i][1].c_str()))
            {
                static IServer *NewServer;
                NewServer = CreateServerInstance(ModuleList[c], (*FileManager.Buffer)[i][0].c_str());

#ifdef _WIN32
                if (NewServer != nullptr)
                {
                    NTServerManager::RegisterServerInterface(NewServer);
                    ServerCount++;
                }
#else
#endif
            }
        }

    }

    fDebugPrint("%s: Loaded %i modules.", __func__, ServerCount);
    return ServerCount;
}
