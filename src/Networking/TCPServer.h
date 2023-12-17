#pragma once

#include <vector>

#include "stdafx.h"
#include "TCPClient.h"

// TODO tidy up includes
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <nunistd.h>
#endif

class CTCPServer
{
public:
    CTCPServer() = default;
    CTCPServer(const uint16_t port);
    ~CTCPServer();

    bool Listen();
    bool AcceptConnection();
    bool RecvPackets();

private:
    SOCKET m_listenSocket { INVALID_SOCKET };

    // TODO more than one client socket
    std::vector<CTCPClient> m_clients;
    
    const uint16_t m_port{ 25565 };
    ESocketState m_listenSocketState { ESocketState::eSS_UNINITIALISED };
};
