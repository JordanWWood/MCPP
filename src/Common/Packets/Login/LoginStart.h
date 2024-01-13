#pragma once

#include "uuid.h"
#include "Packets/IPacket.h"
#include "spdlog/fmt/bin_to_hex.h"

class LoginStart : public IPacket
{
public:
    void Deserialize(char* start) override
    {
        
        uint32_t offset = 0;
        m_username = DeserializeString(start, 16, offset);
        
        uint64_t hiPart = DeserializeULong(start + offset, offset);
        uint64_t loPart = DeserializeULong(start + offset, offset);
        
        m_uuid = CUUID(loPart, hiPart);
    }

    virtual SPacketPayload Serialize() override
    {
        // TODO once we get to proxying we'll want to be able to serialize all packets required for offline connect
        return {};
    }

    std::string m_username;
    CUUID m_uuid;
};
