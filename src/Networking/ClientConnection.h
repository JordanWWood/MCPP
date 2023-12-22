#pragma once

#include "Common\IConnection.h"
#include "SocketUtils.h"

class CClientConnection : public IConnection
{
public:
    CClientConnection(uint64_t socket, std::string socketAddress)
        : m_clientSocket(socket)
        , m_socketAddress(socketAddress)
    {}

    ~CClientConnection() override;

    virtual bool RecvPackets(IPacketHandler* pHandler) final;
    virtual bool SendPacket(SPacketPayload&& payload) final;
    virtual bool IsSocketClosed() const final { return m_socketState == ESocketState::eSS_CLOSED; }
    virtual std::string GetRemoteAddress() const override { return m_socketAddress; }
    
private:
    uint64_t m_clientSocket;

    std::string m_socketAddress;
    ESocketState m_socketState{ ESocketState::eSS_CONNECTED };
};
