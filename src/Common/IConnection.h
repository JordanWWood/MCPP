#pragma once

#include <memory>

struct IPacketHandler;

struct IConnection
{
    virtual ~IConnection() = default;
    virtual bool RecvPackets(IPacketHandler* pHandler) = 0;
    virtual bool IsSocketClosed() const = 0;
};
