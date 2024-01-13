#pragma once

#include "IConnection.h"
#include "SocketUtils.h"
#include "Encryption/SharedSecret.h"
#include "PacketPayload.h"

#include <ConcurrentQueue.h>


class CClientConnection : public IConnection
{
public:
    CClientConnection(uint64_t socket, const std::string& socketAddress)
        : m_clientSocket(socket)
        , m_socketAddress(socketAddress)
    {}

    ~CClientConnection() override;

    /////////////////////////////////////////////////////////////////////
    // IConnection
    virtual std::string GetRemoteAddress() const override { return m_socketAddress; }
    virtual EConnectionType GetConnectionType() const override { return m_type; }
    
    virtual bool RecvPackets(IPacketHandler* pHandler) final;
    virtual void QueuePacket(SPacketPayload&& payload) final;
    virtual bool IsSocketClosed() const final { return m_socketState == ESocketState::eSS_CLOSED; }
    
    virtual void SetAESKey(std::string key) override { m_secret = std::make_unique<CSharedSecret>(std::move(key)); }
    virtual void EnableEncryption() override { m_encryptionEnabled = true; }
    // ~IConnection
    /////////////////////////////////////////////////////////////////////

    bool SendQueuedPackets();
    
private:
    static SPacketPayload ReadUnencryptedPacket(char* start, uint32_t& offset);

    uint64_t m_clientSocket;

    std::string m_socketAddress;
    ESocketState m_socketState{ ESocketState::eSS_CONNECTED };

    std::unique_ptr<CSharedSecret> m_secret;
    bool m_encryptionEnabled { false };

    moodycamel::ConcurrentQueue<SPacketPayload> m_queuedSends;

    const EConnectionType m_type = EConnectionType::eCT_Client;
};
