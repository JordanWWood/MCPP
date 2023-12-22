#pragma once
#include "IPacket.h"

class LoginStart : public IPacket
{
public:
    void Deserialize(char* start) override
    {
        uint32_t offset = 0;
        m_username = DeserializeString(start, 16, offset);
        //DeserializeLong(start + offset, offset);
        //DeserializeLong(start + offset, offset);
    }

    virtual SPacketPayload Serialize() override
    {
        // TODO once we get to proxying we'll want to be able to serialize all packets
        return SPacketPayload();
    }

    std::string m_username;

    uint64_t m_hipart;
    uint64_t m_lopart;
};
