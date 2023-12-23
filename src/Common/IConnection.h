#pragma once

#include <memory>
#include <string>

struct IPacketHandler;
struct SPacketPayload;

struct IConnection
{
    virtual ~IConnection() = default;
    virtual bool RecvPackets(IPacketHandler* pHandler) = 0;
    virtual bool SendPacket(SPacketPayload&& payload) = 0;
    virtual bool IsSocketClosed() const = 0;
    virtual std::string GetRemoteAddress() const = 0;

    virtual void SetAESKey(std::string key) = 0;
    virtual void EnableEncryption() = 0;
};
