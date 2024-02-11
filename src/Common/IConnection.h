#pragma once

#include <memory>
#include <string>

struct IPacketHandler;
struct SPacketPayload;

struct IConnection;
using IConnectionPtr = std::shared_ptr<IConnection>;

enum class EConnectionType
{
    eCT_Client,
    eCT_Server
};

struct IConnection
{
    virtual ~IConnection() = default;
    
    virtual const std::string& GetRemoteAddress() const = 0;
    virtual EConnectionType GetConnectionType() const = 0;
    
    /////////////////////////////////////////////////////
    // Packet/Socket
    virtual bool RecvPackets(IPacketHandler* pHandler) = 0;
    virtual void QueuePacket(SPacketPayload&& payload) = 0;
    virtual bool IsSocketClosed() const = 0;

    /////////////////////////////////////////////////////
    // Encryption/Authentication
    virtual void SetAESKey(std::string key) = 0;
    virtual void EnableEncryption() = 0;
};
