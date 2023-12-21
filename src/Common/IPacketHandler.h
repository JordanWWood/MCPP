#pragma once

struct SPacketPayload;

struct IPacketHandler
{
    virtual ~IPacketHandler() = default;
    virtual bool ProcessPacket(SPacketPayload&& payload) = 0;
};
