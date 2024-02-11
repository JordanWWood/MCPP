#include "pch.h"
#include "TCPSocket.h"

#include "ClientConnection.h"

#define DEFAULT_BUFLEN 512

CTCPSocket::~CTCPSocket()
{
    Stop();
}

CTCPSocket::CTCPSocket(CTCPSocket&& other) noexcept
{
    m_socket = other.m_socket;
    other.m_socket = INVALID_SOCKET;

    m_flags = std::move(other.m_flags);
    m_address = std::move(other.m_address);
    m_port = std::move(other.m_port);
}

bool CTCPSocket::Start()
{
    MCPP_PROFILE_SCOPE()

    // If the socket is already running early out
    if(m_socket != INVALID_SOCKET)
        return true;
    
    addrinfo *result = nullptr;
    
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if(m_flags & eSF_Passive)
        hints.ai_flags = AI_PASSIVE;
    
    int iResult = getaddrinfo(m_flags & eSF_Passive ? nullptr : m_address.c_str(), std::to_string(m_port).c_str(), &hints, &result);
    if(iResult != 0)
    {
        MCLog::error("Failed to create address info for {}:{}", m_address, m_port);
        return false;
    }

    m_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(m_socket == INVALID_SOCKET)
    {
        MCLog::error("Failed to create socket {}:{} with error code {}", m_address, m_port, GET_SOCKET_ERR());
        return false;
    }

#ifdef _WIN32
    u_long iMode = 1;
    if(ioctlsocket(m_socket, FIONBIO, &iMode) != 0)
#else
    int flags = fcntl(m_listenSocket, F_GETFL, 0);
    if (flags == -1)
    {
        MCLog::error("Failed to get flags for socket with error code {}", GET_SOCKET_ERR());
        return false;
    }
    flags = (flags | O_NONBLOCK);
    if(fcntl(m_listenSocket, F_SETFL, flags) != 0)
#endif
    {
        MCLog::error("Failed to change socket {}:{} mode to non-blocking with error code {}", m_address, m_port, GET_SOCKET_ERR());
        Stop();
        return false;
    }

    if(m_flags & eSF_Connect)
    {
        iResult = connect(m_socket, result->ai_addr, static_cast<int>(result->ai_addrlen));
        if(iResult == GET_SOCKET_ERR())
        {
            Stop();
            return false;
        }
    }

    if(m_flags & eSF_Bind)
    {
        iResult = bind(m_socket, result->ai_addr, static_cast<int>(result->ai_addrlen));
        if(iResult == SOCKET_ERROR)
        {
            MCLog::error("Failed to bind to {}:{} with error code {}", m_address, m_port, GET_SOCKET_ERR());
            Stop();
            return false;
        }
    }

    freeaddrinfo(result);

    if(m_flags & eSF_Listen)
    {
        iResult = listen(m_socket, SOMAXCONN);
        if (iResult == SOCKET_ERROR)
        {
            MCLog::error("Failed to listen on {}:{} with error code {}", m_address, m_port, GET_SOCKET_ERR());
            Stop();
            return false;
        }
    }

    MCLog::debug("Successfully started socket {} {}:{}", m_socket, m_address, m_port);

    return true;
}

void CTCPSocket::Stop()
{
    if(m_socket != INVALID_SOCKET)
    {
        MCLog::debug("Closing socket {} {}:{}", m_socket, m_address, m_port);
        
        CLOSE_SOCKET(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

bool CTCPSocket::Send(char* data, const uint32_t size)
{
    assert((m_flags & eSF_Send) && !(m_flags & eSF_Listen));
    
    int iResult = send(m_socket, data, size, 0);
    if (iResult == SOCKET_ERROR)
    {
        MCLog::error("Failed to send payload to client. Error[{}] Address[{}]", GET_SOCKET_ERR(), GetAddress());

        Stop();
        return false;
    }
}

bool CTCPSocket::Recv(TOnPacket&& packetCallback)
{
    MCPP_PROFILE_SCOPE()
    assert((m_flags & eSF_Send) && !(m_flags & eSF_Listen));

    char recvBuffer[DEFAULT_BUFLEN];
    constexpr int recvBufferLength{ DEFAULT_BUFLEN };
    memset(&recvBuffer, 0, recvBufferLength);
    
    const int iResult = recv(m_socket, recvBuffer, recvBufferLength, 0);
    if (iResult > 0)
    {
        packetCallback(recvBuffer, iResult);
        return true;
    }

    if (iResult == 0)
    {
        MCLog::debug("Client disconnected. Address[{}]", GetAddress());
        Stop();
        return true;
    }

    const int error = GET_SOCKET_ERR();
    if(error == WOULD_BLOCK)
    {
        // We're just waiting for something to receive. Break and we'll check if theres something next time
        return true;
    }

    // If we make it here something went wrong
    MCLog::error("Failed to receive packets from client. Error[{}] Address[{}]", error, GetAddress());
    
    Stop();
    return false;
}
