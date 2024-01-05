#pragma once

struct SPacketPayload;

struct IPacketHandler
{
    virtual ~IPacketHandler() = default;
    virtual bool ProcessPacket(SPacketPayload&& payload) = 0;
    virtual void NetworkTick() = 0;
    virtual bool IsDead() const = 0;
};
