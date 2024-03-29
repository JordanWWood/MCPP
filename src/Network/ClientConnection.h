﻿#pragma once

#include "IConnection.h"
#include "Encryption/SharedSecret.h"
#include "PacketPayload.h"

#include <concurrentqueue.h>

#include "TCPSocket.h"


class CClientConnection : public IConnection
{
public:
    CClientConnection(CTCPSocket&& socket)
        : m_socket(std::move(socket))
    {}

    /////////////////////////////////////////////////////////////////////
    // IConnection
    virtual const std::string& GetRemoteAddress() const override { return m_socket.GetAddress(); }
    virtual EConnectionType GetConnectionType() const override { return m_type; }
    
    virtual bool RecvPackets(IPacketHandler* pHandler) final;
    virtual void QueuePacket(SPacketPayload&& payload) final;
    virtual bool IsSocketClosed() const final { return m_socket.IsClosed(); }
    
    virtual void SetAESKey(std::string key) override { m_secret = std::make_unique<CSharedSecret>(std::move(key)); }
    virtual void EnableEncryption() override { m_encryptionEnabled = true; }
    // ~IConnection
    /////////////////////////////////////////////////////////////////////

    bool SendQueuedPackets();
    
private:
    static SPacketPayload ReadUnencryptedPacket(char* start, uint32_t maxSize);
    CTCPSocket m_socket;

    std::unique_ptr<CSharedSecret> m_secret;
    bool m_encryptionEnabled { false };

    moodycamel::ConcurrentQueue<SPacketPayload> m_queuedSends;

    const EConnectionType m_type = EConnectionType::eCT_Client;
};
