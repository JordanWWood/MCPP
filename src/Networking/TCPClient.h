#pragma once
#include <memory>
#include <WinSock2.h>
#include "SocketUtils.h"

#define DEFAULT_BUFLEN 512

struct STCPClientBundle
{
    char m_recvBuffer[DEFAULT_BUFLEN];
    const int m_recvBufferLength{ DEFAULT_BUFLEN };
    
};
using TTCPClientBundlePtr = std::shared_ptr<STCPClientBundle>;

class CTCPClient
{
public:
    CTCPClient(SOCKET socket)
        : m_clientSocket(socket)
    {}
    
    ~CTCPClient();

    TTCPClientBundlePtr RecvPackets();

    ESocketState GetSocketState() const { return m_socketState; }
    
private:
    SOCKET m_clientSocket;
    ESocketState m_socketState{ ESocketState::eSS_CONNECTED };
};
