#pragma once

#include <memory>

#include "SocketUtils.h"
#include "Common/IClient.h"
#include "Packets/BasePacket.h"

class CGameClient : public IClient
{
public:
    CGameClient(uint64_t socket)
        : m_clientSocket(socket)
    {}
    
    ~CGameClient();

    virtual bool RecvPackets() final;
    virtual bool IsSocketClosed() const final { return m_socketState == ESocketState::eSS_CLOSED; }
    
private:
    uint64_t m_clientSocket;
    
    ESocketState m_socketState{ ESocketState::eSS_CONNECTED };
    EClientState m_clientState{ EClientState::eCS_Handshake };
};
