#include "pch.h"
#include "TCPServer.h"

#include "ClientConnection.h"
#include "Encryption/RSAKeyPair.h"

CTCPServer::CTCPServer(const uint16_t port)
    : m_port(port)
{}

CTCPServer::~CTCPServer()
{
    if (m_listenSocket != INVALID_SOCKET)
        closesocket(m_listenSocket);

    WSACleanup();
}

bool CTCPServer::Listen()
{
    OPTICK_EVENT();

#ifdef _WIN32
    WSADATA wsaData;
    int iResult;

    struct addrinfo *result = nullptr;
    struct addrinfo hints;

    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(iResult != 0)
    {
        // TODO Logging
        return false;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(nullptr, std::to_string(m_port).c_str(), &hints, &result);
    if(iResult != 0)
    {
        m_listenSocketState = ESocketState::eSS_INVALID_ADDR;

        MCLog::error("Error retrieving address info while creating listen socket");
        WSACleanup();
        return false;
    }
    
    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(m_listenSocket == INVALID_SOCKET)
    {
        m_listenSocketState = ESocketState::eSS_INVALID;

        MCLog::error("Error retrieving address info while creating listen socket");
        WSACleanup();
        return false;
    }

    u_long iMode = 1;
    if(ioctlsocket(m_listenSocket, FIONBIO, &iMode) != 0)
    {
        closesocket(m_listenSocket);
        
        MCLog::error("Failed to change listen socket mode to non-blocking with error code {}", WSAGetLastError());
        WSACleanup();
        return false;
    }

    iResult = bind(m_listenSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if(iResult == SOCKET_ERROR)
    {
        m_listenSocketState = ESocketState::eSS_BIND_ERROR;
        
        MCLog::error("Failed to bind to port {} with error code {}", m_port, WSAGetLastError());
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    freeaddrinfo(result);

    iResult = listen(m_listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        m_listenSocketState = ESocketState::eSS_LISTEN_ERROR;
        
        MCLog::error("Failed to listen on port {} with error code {}", m_port, WSAGetLastError());
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    MCLog::info("Successfully started listening on {}", m_port);

    m_listenSocketState = ESocketState::eSS_LISTEN;
#endif

    return true;
}

IConnectionPtr CTCPServer::AcceptConnection() const
{
    OPTICK_EVENT();

    struct sockaddr_in sa = { 0 }; /* for TCP/IP */
    socklen_t socklen = sizeof sa;

    SOCKET socket = accept(m_listenSocket, reinterpret_cast<struct sockaddr*>(&sa), &socklen);
    if(socket == INVALID_SOCKET)
    {
        if(WSAGetLastError() == WSAEWOULDBLOCK)
            return nullptr;

        MCLog::error("Failed to accept new client connect with error {}", WSAGetLastError());

        closesocket(m_listenSocket);
        return nullptr;
    }

    MCLog::info("Accepting connection from {}", inet_ntoa(sa.sin_addr));
    
    return std::make_shared<CClientConnection>(socket, inet_ntoa(sa.sin_addr));
}
