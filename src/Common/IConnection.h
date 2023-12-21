#pragma once

#include <memory>

struct IPacketHandler;
struct SPacketPayload;

struct IConnection
{
    virtual ~IConnection() = default;
    virtual bool RecvPackets(IPacketHandler* pHandler) = 0;
    virtual bool SendPacket(SPacketPayload&& payload) = 0;
    virtual bool IsSocketClosed() const = 0;
};
