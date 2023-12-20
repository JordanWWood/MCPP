#pragma once

#include <vector>

#include "stdafx.h"
#include "GameClient.h"

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

using IClientPtr = std::shared_ptr<IClient>;

class CTCPServer
{
public:
    CTCPServer(const uint16_t port);
    ~CTCPServer();

    // Initialise a port and listen on said port for incoming connections
    bool Listen();

    // Check for any incoming connections, accept and create a port if there are any
    IClientPtr AcceptConnection() const;
    bool IsSocketClosed() const { return m_listenSocketState == ESocketState::eSS_CLOSED; }

private:
    SOCKET m_listenSocket { INVALID_SOCKET };
    ESocketState m_listenSocketState { ESocketState::eSS_UNINITIALISED };

    const uint16_t m_port{ 25565 };
};
