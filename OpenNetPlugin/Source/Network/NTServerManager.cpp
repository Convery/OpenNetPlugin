/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-16
    Notes:
        Winsock replacement methods for the local servers.
        If a packet is not handled, it's forwarded to winsock.
*/

#include "NTServerManager.h"
#include "..\Interfaces\IServer.h"
#include "..\Interfaces\IService.h"

// Internal mappings of servers.
std::unordered_map<uint64_t, uint32_t>          NTServerManager::Host_ProxyAddresses;
std::unordered_map<uint64_t, class IServer *>   NTServerManager::Host_ServerAddresses;
std::unordered_map<void *, class IServer *>     NTServerManager::Host_ConnectedSockets;
std::unordered_map<void *, bool>                NTServerManager::Host_SocketBlockStatus;

// Map iteration.
IServer *NTServerManager::FindServerBySocket(void *Socket)
{
    auto Iterator = Host_ConnectedSockets.find(Socket);

    if (Iterator != Host_ConnectedSockets.end())
        return Iterator->second;
    else
        return nullptr;
}
IServer *NTServerManager::FindServerByAddress(uint64_t IPv6Address)
{
    auto Iterator = Host_ServerAddresses.find(IPv6Address);

    if (Iterator != Host_ServerAddresses.end())
        return Iterator->second;
    else
        return nullptr;
}
IServer *NTServerManager::FindServerByHost(const char *Hostname)
{
    uint32_t InternalAddressv4;
    uint64_t InternalAddressv6;
    IServer *Result = nullptr;

    InternalAddressv4 = 0x0000000A | FNV1_32Hash((void *)Hostname, (uint32_t)strlen(Hostname)) << 8;
    InternalAddressv6 = 0x00000000000000FD | static_cast<uint64_t>(FNV1_32Hash((void *)Hostname, (uint32_t)strlen(Hostname))) << 8;

    Result = FindServerByAddress(InternalAddressv4);
    return Result == nullptr ? FindServerByAddress(InternalAddressv6) : Result;
}
