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
        CLOSE_SOCKET(m_listenSocket);

#ifdef _WIN32
    WSACleanup();
#endif
}

bool CTCPServer::Listen()
{
    MCPP_PROFILE_SCOPE()
    
    int iResult;

    addrinfo *result = nullptr;
    addrinfo hints;

#ifdef _WIN32
    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(iResult != 0)
    {
        // TODO Logging
        return false;
    }
#endif

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(nullptr, std::to_string(m_port).c_str(), &hints, &result);
    if(iResult != 0)
    {
        m_listenSocketState = ESocketState::eSS_INVALID_ADDR;

        MCLog::error("Error retrieving address info while creating listen socket");
        CLEANUP_NETWORK();
        return false;
    }
    
    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(m_listenSocket == INVALID_SOCKET)
    {
        m_listenSocketState = ESocketState::eSS_INVALID;

        MCLog::error("Error retrieving address info while creating listen socket");
        CLEANUP_NETWORK();
        return false;
    }

#ifdef _WIN32
    u_long iMode = 1;
    if(ioctlsocket(m_listenSocket, FIONBIO, &iMode) != 0)
#else
    int flags = fcntl(m_listenSocket, F_GETFL, 0);
    if (flags == -1)
    {
        MCLog::error("Failed to get flags for socket with error code {}", GET_SOCKET_ERR());
        CLEANUP_NETWORK();
        return false;
    }
    flags = (flags | O_NONBLOCK);
    if(fcntl(m_listenSocket, F_SETFL, flags) == 0)
#endif
    {
        CLOSE_SOCKET(m_listenSocket);
        
        MCLog::error("Failed to change listen socket mode to non-blocking with error code {}", GET_SOCKET_ERR());
        CLEANUP_NETWORK();
        return false;
    }

    iResult = bind(m_listenSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if(iResult == SOCKET_ERROR)
    {
        m_listenSocketState = ESocketState::eSS_BIND_ERROR;
        
        MCLog::error("Failed to bind to port {} with error code {}", m_port, GET_SOCKET_ERR());
        CLOSE_SOCKET(m_listenSocket);
        CLEANUP_NETWORK();
        return false;
    }

    freeaddrinfo(result);

    iResult = listen(m_listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        m_listenSocketState = ESocketState::eSS_LISTEN_ERROR;
        
        MCLog::error("Failed to listen on port {} with error code {}", m_port, GET_SOCKET_ERR());
        CLOSE_SOCKET(m_listenSocket);
        CLEANUP_NETWORK();
        return false;
    }

    MCLog::info("Successfully started listening on {}", m_port);

    m_listenSocketState = ESocketState::eSS_LISTEN;

    return true;
}

IConnectionPtr CTCPServer::AcceptConnection() const
{
    MCPP_PROFILE_SCOPE();

    sockaddr_in sa = { 0 }; /* for TCP/IP */
    socklen_t socklen = sizeof sa;

    SOCKET socket = accept(m_listenSocket, reinterpret_cast<struct sockaddr*>(&sa), &socklen);
    if(socket == INVALID_SOCKET)
    {
        
        if(GET_SOCKET_ERR() == WOULD_BLOCK)
            return nullptr;

        MCLog::error("Failed to accept new client connect with error {}", GET_SOCKET_ERR());

        CLOSE_SOCKET(m_listenSocket);
        return nullptr;
    }

    MCLog::info("Accepting connection from {}", inet_ntoa(sa.sin_addr));
    
    return std::make_shared<CClientConnection>(socket, inet_ntoa(sa.sin_addr));
}
