#pragma once

#include "IConnection.h"
#include "SocketUtils.h"
#include "Encryption/SharedSecret.h"

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
    
    virtual void SetAESKey(std::string key) override { m_secret = std::make_unique<CSharedSecret>(std::move(key)); }
    virtual void EnableEncryption() override { m_encryptionEnabled = true; }
    
private:
    SPacketPayload ReadUnencryptedPacket(char* start, uint32_t& offset);
    
    uint64_t m_clientSocket;

    std::string m_socketAddress;
    ESocketState m_socketState{ ESocketState::eSS_CONNECTED };

    std::unique_ptr<CSharedSecret> m_secret;
    bool m_encryptionEnabled { false };
};
