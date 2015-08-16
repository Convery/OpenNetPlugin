/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-16
    Notes:
        Winsock replacement methods for the local servers.
        If a packet is not handled, it's forwarded to winsock.
*/

#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "..\STDInclude.h"
#include <WinSock2.h>
#include <unordered_map>
#pragma comment(lib, "ws2_32.lib")	

class NTServerManager
{
    // Internal mappings of servers.
    static std::unordered_map<uint64_t /*ServerIPv6*/, uint32_t /*ServerIPv4*/>     Host_ProxyAddresses;
    static std::unordered_map<uint16_t /*Old port*/, uint16_t /*New port*/>         Host_ProxyPorts;
    static std::unordered_map<uint64_t /*ServerIPv6*/, class IServer *>             Host_ServerAddresses;
    static std::unordered_map<void */*Socket*/, class IServer *>                    Host_ConnectedSockets;
    static std::unordered_map<void */*Socket*/, bool /*Blocked*/>                   Host_SocketBlockStatus;

    // Map iteration.
    static class IServer *FindServerBySocket(void *Socket);
    static class IServer *FindServerByAddress(uint64_t IPv6Address);
    static class IServer *FindServerByHost(const char *Hostname);

    // Old winsock replacement methods.
    static void *__stdcall      NT_Accept(void *Socket, sockaddr *Address, int32_t *AddressLength);
    static int32_t __stdcall    NT_Bind(void *Socket, const sockaddr *Address, int32_t AddressLength);
    static int32_t __stdcall    NT_CloseSocket(void *Socket);
    static int32_t __stdcall    NT_Connect(void *Socket, const sockaddr *Address, int32_t AddressLength);
    static int32_t __stdcall    NT_GetPeerName(void *Socket, sockaddr *Address, int32_t *AddressLength);
    static int32_t __stdcall    NT_GetSockName(void *Socket, sockaddr *Address, int32_t *AddressLength);
    static int32_t __stdcall    NT_GetSockOpt(void *Socket, int32_t Level, int32_t OptionName, char *OptionValue, int32_t *OptionLength);
    static int32_t __stdcall    NT_IOControlSocket(void *Socket, int32_t Command, u_long *ArgumentPointer);
    static int32_t __stdcall    NT_Listen(void *Socket, int32_t Backlog);
    static int32_t __stdcall    NT_Receive(void *Socket, char *Buffer, int32_t BufferLength, int32_t Flags);
    static int32_t __stdcall    NT_ReceiveFrom(void *Socket, char *Buffer, int32_t BufferLength, int32_t Flags, sockaddr *Peer, int32_t *PeerLength);
    static int32_t __stdcall    NT_Select(int32_t fdsCount, fd_set *Readfds, fd_set *Writefds, fd_set *Exceptfds, const timeval *Timeout);
    static int32_t __stdcall    NT_Send(void *Socket, const char *Buffer, int32_t BufferLength, int32_t Flags);
    static int32_t __stdcall    NT_SendTo(void *Socket, const char *Buffer, int32_t BufferLength, int32_t Flags, const sockaddr *Peer, int32_t PeerLength);
    static int32_t __stdcall    NT_SetSockOpt(void *Socket, int32_t Level, int32_t OptionName, const char *OptionValue, int32_t OptionLength);
    static hostent *__stdcall   NT_GetHostByName(const char *Hostname);

    // New winsock replacement methods.
    // TODO: The WSA* methods.

public:
    // Initialization.
    static bool InitializeImportHooks();
    static bool StartProcessingPackets();
    static void PacketProcessingThread();

    // Map modification from usercode, returns the index of a server.
    static uint64_t RegisterProxyserver(const char *Hostname, uint32_t IPv4Address);
    static uint64_t RegisterServerInterface(class IServer *Server);
    static void    RegisterService(uint64_t ServerIPv6, class IService *Service);
};
