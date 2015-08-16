/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-16
    Notes:
        Servers provide their own queue for IO.
        Servers can be identified by IP or hostname.
*/

#pragma once
#include "..\Utility\Cryptography.h"
#include <stdint.h>
#include <cstring>

// The base interface for local servers.
class IServer
{
protected:
    // Represents an IP on the internal network.
    uint32_t InternalAddress4;
    uint64_t InternalAddress6;
    const char *Hostname;

    // Basic server management.
    virtual void    Platform_Select(int32_t *ReadCount, void **SocketsRead, int32_t *WriteCount, void **SocketsWrite) = 0;
    virtual int32_t Platform_Receive(uint8_t *Buffer, int32_t Length, void *Socket = nullptr) = 0;
    virtual int32_t Platform_Send(uint8_t *Buffer, int32_t Length, void *Socket = nullptr) = 0;
    virtual int32_t Platform_Connect(void *Socket, void *NameStruct, int32_t NameLength) = 0;
    virtual int32_t Platform_Disconnect(void *Socket) = 0;

    // Internal server management.
    virtual void Internal_ResetServer() = 0;
    virtual void Internal_RunServerFrame() = 0;
    virtual void Internal_RegisterService(class IService *Service) = 0;

    // Expose platform_ methods to the platform manager.
    friend class NTServerManager;
    friend class NIXServerManager;

public:
    // Methods called from usercode.
    virtual int32_t User_EnqueueData(int32_t Length, uint8_t *Buffer = nullptr) = 0;
    virtual int32_t User_DequeueData(int32_t Length, uint8_t *Buffer = nullptr) = 0;

    // Create the address.
    IServer() noexcept;
    IServer(const char *Hostname) noexcept;
};

// Create unique and invalid addresses.
inline IServer::IServer() noexcept
{
    InternalAddress4 = 0x0000000A;
    InternalAddress6 = 0x00000000000000FD;
}
inline IServer::IServer(const char *Hostname) noexcept
{
    InternalAddress4 = 0x0000000A | FNV1_32Hash((void *)Hostname, (uint32_t)strlen(Hostname)) << 8;
    InternalAddress6 = 0x00000000000000FD | static_cast<uint64_t>(FNV1_32Hash((void *)Hostname, (uint32_t)strlen(Hostname))) << 8;
}
