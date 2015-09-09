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
#include "..\STDInclude.h"
#include <thread>
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp.lib")

// Internal mappings of servers.
std::unordered_map<uint64_t, uint32_t>          NTServerManager::Host_ProxyAddresses;
std::unordered_map<uint16_t, uint16_t>          NTServerManager::Host_ProxyPorts;
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
IServer *NTServerManager::FindServerByAddress(uint64_t IPAddress)
{
    for (auto Iterator = Host_ServerAddresses.begin(); Iterator != Host_ServerAddresses.end(); ++Iterator)
    {
        if (IPAddress == Iterator->second->InternalAddress4) return Iterator->second;
        if (IPAddress == Iterator->second->InternalAddress6) return Iterator->second;
    }

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

// Old winsock replacement methods.
void *NTServerManager::NT_Accept(void *Socket, sockaddr *Address, int32_t *AddressLength)
{
    return (void *)accept((SOCKET)Socket, Address, AddressLength);
}
int32_t NTServerManager::NT_Bind(void *Socket, const sockaddr *Address, int32_t AddressLength)
{
    if (AddressLength == sizeof(sockaddr_in))
    {
        nDebugPrint("%s to address %s:%u on socket %p", __func__, inet_ntoa(((sockaddr_in *)Address)->sin_addr), ntohs(((sockaddr_in *)Address)->sin_port), Socket);
    }

    return bind((SOCKET)Socket, Address, AddressLength);
}
int32_t NTServerManager::NT_CloseSocket(void *Socket)
{
    IServer *Server = FindServerBySocket(Socket);

    if (Server != nullptr)
    {
        nDebugPrint("%s for server \"%s\"", __func__, Server->Hostname);

        Host_ConnectedSockets.erase(Socket);
        Server->Platform_Disconnect(Socket);
    }

    return closesocket((SOCKET)Socket);
}
int32_t NTServerManager::NT_Connect(void *Socket, const sockaddr *Address, int32_t AddressLength)
{
    if (AddressLength == sizeof(sockaddr_in))
    {
        sockaddr_in *IPAddress = (sockaddr_in *)Address;
        IServer *Server = FindServerByAddress(IPAddress->sin_addr.s_addr);

        // Connect to a local server.
        if (Server != nullptr)
        {
            Host_ConnectedSockets[Socket] = Server;
            Server->Platform_Connect(Socket, (void *)Address, AddressLength);

            if (!Host_SocketBlockStatus[Socket])
            {
                return 0;
            }
        }

        // Replace the port if it's on localhost.
        if (IPAddress->sin_addr.s_addr == inet_addr("127.0.0.1"))
        {
            auto Iterator = Host_ProxyPorts.find(ntohs(((sockaddr_in *)Address)->sin_port));

            if (Iterator != Host_ProxyPorts.end())
            {
                nDebugPrint("%s replaces port %u with %u", __func__, ntohs(((sockaddr_in *)Address)->sin_port), Iterator->second);
                ((sockaddr_in *)Address)->sin_port = htons(Iterator->second);
            }
        }

        nDebugPrint("%s to address %s:%u on socket %p", __func__, inet_ntoa(((sockaddr_in *)Address)->sin_addr), ntohs(((sockaddr_in *)Address)->sin_port), Socket);
    }

    return connect((SOCKET)Socket, Address, AddressLength);
}
int32_t NTServerManager::NT_GetPeerName(void *Socket, sockaddr *Address, int32_t *AddressLength)
{
    return getpeername((SOCKET)Socket, Address, AddressLength);
}
int32_t NTServerManager::NT_GetSockName(void *Socket, sockaddr *Address, int32_t *AddressLength)
{
    return getsockname((SOCKET)Socket, Address, AddressLength);
}
int32_t NTServerManager::NT_GetSockOpt(void *Socket, int32_t Level, int32_t OptionName, char *OptionValue, int32_t *OptionLength)
{
    return getsockopt((SOCKET)Socket, Level, OptionName, OptionValue, OptionLength);
}
int32_t NTServerManager::NT_IOControlSocket(void *Socket, uint32_t Command, u_long *ArgumentPointer)
{
    const char *ReadableCommand = "UNKNOWN";

    switch (Command)
    {
        case FIONBIO: ReadableCommand = "FIONBIO"; 
            // Set the blocking status.
            Host_SocketBlockStatus[Socket] = *ArgumentPointer == 0;
            break;
        case FIONREAD: ReadableCommand = "FIONREAD"; break;
        case FIOASYNC: ReadableCommand = "FIOASYNC"; break;

        case SIOCSHIWAT: ReadableCommand = "SIOCSHIWAT"; break;
        case SIOCGHIWAT: ReadableCommand = "SIOCGHIWAT"; break;
        case SIOCSLOWAT: ReadableCommand = "SIOCSLOWAT"; break;
        case SIOCGLOWAT: ReadableCommand = "SIOCGLOWAT"; break;
        case SIOCATMARK: ReadableCommand = "SIOCATMARK"; break;
    }

    nDebugPrint("%s on socket %p with command \"%s\"", __func__, Socket, ReadableCommand);
    return ioctlsocket((SOCKET)Socket, Command, ArgumentPointer);
}
int32_t NTServerManager::NT_Listen(void *Socket, int32_t Backlog)
{
    return listen((SOCKET)Socket, Backlog);
}
int32_t NTServerManager::NT_Receive(void *Socket, char *Buffer, int32_t BufferLength, int32_t Flags)
{
    IServer *Server = FindServerBySocket(Socket);
    int32_t BytesReceived = -1;

    // Local server.
    if (Server != nullptr)
    {
        // If we are blocking, block until we have data to send.
        if (Host_SocketBlockStatus[Socket])
        {
            while (BytesReceived == -1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                BytesReceived = Server->Platform_Receive((uint8_t *)Buffer, BufferLength, Socket);
            }
        }
        else
        {
            BytesReceived = Server->Platform_Receive((uint8_t *)Buffer, BufferLength, Socket);
            if (BytesReceived == -1)
            {
                WSASetLastError(WSAEWOULDBLOCK);
            }
        }

        // Log on data received.
        if (BytesReceived != -1)
        {
            nDebugPrint("%s: %i bytes from %s", __func__, BytesReceived, Server->Hostname);
        }

        return BytesReceived;
    }
    
    // Else receive from winsock.
    BytesReceived = recv((SOCKET)Socket, Buffer, BufferLength, Flags);
    if (BytesReceived != -1)
    {
        nDebugPrint("%s: %i bytes on socket %p", __func__, BytesReceived, Socket);
    }

    return BytesReceived;
}
int32_t NTServerManager::NT_ReceiveFrom(void *Socket, char *Buffer, int32_t BufferLength, int32_t Flags, sockaddr *Peer, int32_t *PeerLength)
{
    IServer *Server = FindServerBySocket(Socket);
    int32_t BytesReceived = -1;

    // Local server.
    if (Server != nullptr)
    {
        // LocalStorage is filled in the sendto method.
        memcpy(Peer, Server->LocalStorage, sizeof(sockaddr));

        // If we are blocking, block until we have data to send.
        if (Host_SocketBlockStatus[Socket])
        {
            while (BytesReceived == -1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                BytesReceived = Server->Platform_Receive((uint8_t *)Buffer, BufferLength, Socket);
            }
        }
        else
        {
            BytesReceived = Server->Platform_Receive((uint8_t *)Buffer, BufferLength, Socket);
            if (BytesReceived == -1)
            {
                WSASetLastError(WSAEWOULDBLOCK);
            }
        }

        // Log on data received.
        if (BytesReceived != -1)
        {
            nDebugPrint("%s: %i bytes from %s", __func__, BytesReceived, Server->Hostname);
        }

        return BytesReceived;
    }

    // Else receive from winsock.
    BytesReceived = recvfrom((SOCKET)Socket, Buffer, BufferLength, Flags, Peer, PeerLength);
    if (BytesReceived != -1)
    {
        nDebugPrint("%s: %i bytes on socket %p", __func__, BytesReceived, Socket);
    }

    return BytesReceived;
}
int32_t NTServerManager::NT_Select(int32_t fdsCount, fd_set *Readfds, fd_set *Writefds, fd_set *Exceptfds, const timeval *Timeout)
{
    int32_t SocketCount;
    int32_t ReadCount, WriteCount;
    void *ReadSockets[20], *WriteSockets[20];

    // Request socket info from winsock.
    SocketCount = select(fdsCount, Readfds, Writefds, Exceptfds, Timeout);
    if (SocketCount == -1)
        return SocketCount;

    // Add our servers.
    for (auto Iterator = Host_ConnectedSockets.begin(); Iterator != Host_ConnectedSockets.end(); ++Iterator)
    {
        WriteCount = 10; ReadCount = 10;
        Iterator->second->Platform_Select(&ReadCount, ReadSockets, &WriteCount, WriteSockets);
    
        if (Readfds)
        {
            for (int32_t i = 0; i < ReadCount; ++i)
            {
                FD_SET((SOCKET)ReadSockets[i], Readfds);
                SocketCount++;
            }
            for (int32_t i = 0; i < WriteCount; ++i)
            {
                FD_SET((SOCKET)WriteSockets[i], Writefds);
                SocketCount++;
            }
        }
    }

    // Debug info.
    // nDebugPrint("%s returned %u results", __func__, SocketCount);
    return SocketCount;
}
int32_t NTServerManager::NT_Send(void *Socket, const char *Buffer, int32_t BufferLength, int32_t Flags)
{
    IServer *Server = FindServerBySocket(Socket);

    if (Server != nullptr)
    {
        nDebugPrint("%s to \"%s\", %i bytes.", __func__, Server->Hostname, BufferLength);
        return Server->Platform_Send((uint8_t *)Buffer, BufferLength, Socket);
    }
    else
    {
        nDebugPrint("%s via %p, %i bytes.", __func__, Socket, BufferLength);
        return send((SOCKET)Socket, Buffer, BufferLength, Flags);
    }
}
int32_t NTServerManager::NT_SendTo(void *Socket, const char *Buffer, int32_t BufferLength, int32_t Flags, const sockaddr *Peer, int32_t PeerLength)
{
    IServer *Server = FindServerBySocket(Socket);
    int32_t BytesSent = -1;

    if (Server != nullptr)
    {
        // Emplace the server.
        Host_ConnectedSockets[Socket] = Server;
        memcpy(&Server->LocalStorage[SERVER_HOST_INFO], Peer, sizeof(sockaddr));

        nDebugPrint("%s to \"%s\", %i bytes.", __func__, Server->Hostname, BufferLength);
        return Server->Platform_Send((uint8_t *)Buffer, BufferLength, Socket);
    }
    else
    {
        nDebugPrint("%s via %p, %i bytes.", __func__, Socket, BufferLength);
        return sendto((SOCKET)Socket, Buffer, BufferLength, Flags, Peer, PeerLength);
    }
}
int32_t NTServerManager::NT_SetSockOpt(void *Socket, int32_t Level, int32_t OptionName, const char *OptionValue, int32_t OptionLength)
{
    return setsockopt((SOCKET)Socket, Level, OptionName, OptionValue, OptionLength);
}
hostent *NTServerManager::NT_GetHostByName(const char *Hostname)
{
    IServer *Server = FindServerByHost(Hostname);
    auto Iterator = Host_ProxyAddresses.find(0x00000000000000FD | static_cast<uint64_t>(FNV1_32Hash((void *)Hostname, (uint32_t)strlen(Hostname))) << 8);

    if (Server != nullptr || Iterator != Host_ProxyAddresses.end())
    {
        static in_addr LocalAddress;
        if (Server != nullptr)
        {
            LocalAddress.s_addr = Server->InternalAddress4;
        }
        else
        {
            LocalAddress.s_addr = Iterator->second;
        }

        static in_addr *LocalSocketAddrList[2];
        LocalSocketAddrList[0] = &LocalAddress;
        LocalSocketAddrList[1] = nullptr;

        static hostent LocalHost;
        LocalHost.h_name = const_cast<char *>(Hostname);
        LocalHost.h_aliases = NULL;
        LocalHost.h_addrtype = AF_INET;
        LocalHost.h_length = sizeof(in_addr);
        LocalHost.h_addr_list = (char **)LocalSocketAddrList;

        nDebugPrint("%s: \"%s\" -> %s", __func__, Hostname, inet_ntoa(*(in_addr*)LocalHost.h_addr_list[0]));
        return &LocalHost;
    }
    else
    {
        static hostent *ResolvedHost = nullptr;
        ResolvedHost = gethostbyname(Hostname);

        if (ResolvedHost != nullptr)
        {
            nDebugPrint("%s: \"%s\" -> %s", __func__, Hostname, inet_ntoa(*(in_addr*)ResolvedHost->h_addr_list[0]));
        }

        return ResolvedHost;
    }
}

// Initialization.
bool NTServerManager::InitializeImportHooks()
{
#define PATCH_WINSOCK_IAT(name, function) \
    PatchCount += ReplaceIATEntry("wsock32.dll", name, function) ? 1 : 0; \
    PatchCount += ReplaceIATEntry("WS2_32.dll", name, function) ? 1 : 0; 

    uint32_t PatchCount = 0;

    PATCH_WINSOCK_IAT("accept", NT_Accept);
    PATCH_WINSOCK_IAT("bind", NT_Bind);
    PATCH_WINSOCK_IAT("closesocket", NT_CloseSocket);
    PATCH_WINSOCK_IAT("connect", NT_Connect);
    PATCH_WINSOCK_IAT("getpeername", NT_GetPeerName);
    PATCH_WINSOCK_IAT("getsockname", NT_GetSockName);
    PATCH_WINSOCK_IAT("getsockopt", NT_GetSockOpt);
    PATCH_WINSOCK_IAT("ioctlsocket", NT_IOControlSocket);
    PATCH_WINSOCK_IAT("listen", NT_Listen);
    PATCH_WINSOCK_IAT("recv", NT_Receive);
    PATCH_WINSOCK_IAT("recvfrom", NT_ReceiveFrom);
    PATCH_WINSOCK_IAT("select", NT_Select);
    PATCH_WINSOCK_IAT("send", NT_Send);
    PATCH_WINSOCK_IAT("sendto", NT_SendTo);
    PATCH_WINSOCK_IAT("setsockopt", NT_SetSockOpt);
    PATCH_WINSOCK_IAT("gethostbyname", NT_GetHostByName);

    return PatchCount > 0;
}
bool NTServerManager::StartProcessingPackets()
{
    static std::thread PacketThread;

    if (!PacketThread.joinable())
    {
        PacketThread = std::thread(PacketProcessingThread);
        PacketThread.detach();
        return true;
    }

    return false;
}
void NTServerManager::PacketProcessingThread()
{
    // A ping of < 30ms should be acceptable.
    std::chrono::milliseconds SleepDuration(30);

    while (true)
    {
        std::this_thread::sleep_for(SleepDuration);

        for (auto Iterator = Host_ServerAddresses.begin(); Iterator != Host_ServerAddresses.end(); ++Iterator)
        {
            try
            {
                Iterator->second->Internal_RunServerFrame();
            }
            catch (const std::exception &Exception)
            {
                DebugPrint("\n\n\n==========================================================");
                DebugPrint("OpenNet error");
                DebugPrint(Exception.what());
                DebugPrint("\n=============================================\n");

                DebugPrint("One of the local socket processes threw an unhandled exception.");
                DebugPrint("The plugin may still work in offline mode but should be restarted.");

                HANDLE hFile = INVALID_HANDLE_VALUE;
                hFile = CreateFileA("OpenNetCrash.dmp", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    if(strstr(GetCommandLineA(), "-handledump"))
                        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithHandleData, NULL, NULL, NULL);
                    else
                        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, NULL, NULL, NULL);

                    if (hFile != INVALID_HANDLE_VALUE)
                        CloseHandle(hFile);
                }
            }
        }
    }
}

// Map modification from usercode, returns the index of a server.
uint64_t NTServerManager::RegisterProxyserver(const char *Hostname, uint32_t IPv4Address)
{
    uint64_t InternalAddress = 0x00000000000000FD | static_cast<uint64_t>(FNV1_32Hash((void *)Hostname, (uint32_t)strlen(Hostname))) << 8;

    Host_ProxyAddresses[InternalAddress] = IPv4Address;
    return InternalAddress;
}
uint64_t NTServerManager::RegisterServerInterface(IServer *Server)
{
    Host_ServerAddresses.emplace(Server->InternalAddress6, Server);
    return Server->InternalAddress6;
}
void    NTServerManager::RegisterService(uint64_t ServerIPv6, IService *Service)
{
    Host_ServerAddresses[ServerIPv6]->Internal_RegisterService(Service);
}
