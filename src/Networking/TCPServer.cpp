#include "TCPServer.h"

#include <string>
#pragma comment(lib, "Ws2_32.lib")

CTCPServer::CTCPServer(const uint16_t port)
    : m_port(port)
{}

CTCPServer::~CTCPServer()
{
    if (m_listenSocket != INVALID_SOCKET)
        closesocket(m_listenSocket);
}

bool CTCPServer::Listen()
{
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

        // TODO logging
        WSACleanup();
        return false;
    }
    
    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(m_listenSocket == INVALID_SOCKET)
    {
        m_listenSocketState = ESocketState::eSS_INVALID;

        // TODO logging
        WSACleanup();
        return false;
    }

    u_long iMode = 1;
    if(ioctlsocket(m_listenSocket, FIONBIO, &iMode) != 0)
    {
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    iResult = bind(m_listenSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if(iResult == SOCKET_ERROR)
    {
        m_listenSocketState = ESocketState::eSS_BIND_ERROR;
        
        // TODO logging
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    freeaddrinfo(result);

    iResult = listen(m_listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        m_listenSocketState = ESocketState::eSS_LISTEN_ERROR;
        
        // TODO logging
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    m_listenSocketState = ESocketState::eSS_LISTEN;
#endif

    return true;
}

bool CTCPServer::AcceptConnection()
{
    printf("Waiting for connection to accept\n");

    SOCKET socket = INVALID_SOCKET;
    
    socket = accept(m_listenSocket, nullptr, nullptr);
    if(socket == INVALID_SOCKET)
    {
        if(WSAGetLastError() == WSAEWOULDBLOCK)
        {
            printf("WSAEWOULDBLOCK\n");
            return false;
        }
        // TOOD logging

        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    m_clients.emplace_back(socket);
    
    return true;
}

bool CTCPServer::RecvPackets()
{
    for (std::vector<CTCPClient>::iterator it = m_clients.begin(); it != m_clients.end(); )
    {
        CTCPClient& client = *it;
        TTCPClientBundlePtr bundle = client.RecvPackets();

        if(client.GetSocketState() == ESocketState::eSS_CLOSED)
        {
            it = m_clients.erase(it); // This socket is finished. Remove it
            continue;
        }

        ++it;
    }

    return true;
}
