#pragma once

#include <memory>

#include "Common\IConnection.h"
#include "Common\IPacketHandler.h"
#include "SocketUtils.h"

class CClientConnection : public IConnection
{
public:
    CClientConnection(uint64_t socket)
        : m_clientSocket(socket)
    {}

    ~CClientConnection() override;

    virtual bool RecvPackets(IPacketHandler* pHandler) final;
    virtual bool IsSocketClosed() const final { return m_socketState == ESocketState::eSS_CLOSED; }
    
private:
    uint64_t m_clientSocket;
    
    ESocketState m_socketState{ ESocketState::eSS_CONNECTED };
};
