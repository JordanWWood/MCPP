#pragma once
#include "IPacket.h"
#include "Common/PacketPayload.h"

struct SLoginSuccess : public IPacket
{
    // TODO for connecting to servers
    virtual void Deserialize(char* start) override;

    virtual SPacketPayload Serialize() override
    {
        
    }
};
