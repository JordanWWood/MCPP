#pragma once
#include <cstdint>

#include "IPacket.h"
#include "Common/PacketPayload.h"

struct SHandshake final : public IPacket
{
    virtual void Deserialize(char* start) override
    {
        uint32_t offset = 0;
        m_protocolVersion = DeserializeVarInt(start, offset);
        m_address = DeserializeString(&start[offset], 32, offset);
        m_port = DeserializeShort(&start[offset], offset);
        m_nextState = static_cast<uint8_t>(start[offset]);
    }

    virtual SPacketPayload Serialize() override
    {
        // TODO
        return SPacketPayload();
    }
    
    uint32_t m_protocolVersion { 0 };
    std::string m_address;
    uint16_t m_port { 0 };
    uint8_t m_nextState { 0 };
};
