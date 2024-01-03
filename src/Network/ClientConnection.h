#pragma once

#include "IConnection.h"
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
    
    virtual void SetAESKey(std::string key) override { m_aesKey = key; }
    virtual void EnableEncryption() override { m_encryptionEnabled = true; }

    virtual std::string GenerateHexDigest(std::string publicKey, std::string sharedSecret) override;
    
private:
    SPacketPayload ReadUnencryptedPacket(char* start, uint32_t& offset);
    unsigned char* DecryptPacket(unsigned char* start, int length) const;
    unsigned char* EncryptPacket(unsigned char* start, int length, int& outCipherLength) const;
    
    uint64_t m_clientSocket;

    std::string m_socketAddress;
    ESocketState m_socketState{ ESocketState::eSS_CONNECTED };

    std::string m_aesKey;
    bool m_encryptionEnabled { false };
};
