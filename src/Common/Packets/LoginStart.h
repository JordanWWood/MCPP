#pragma once
#include "IPacket.h"

class LoginStart : public IPacket
{
public:
    void Deserialize(char* start) override
    {
        uint32_t offset;
        m_username = DeserializeString(start, 16, offset);
        //DeserializeLong(start + offset, offset);
        //DeserializeLong(start + offset, offset);
    }

    std::string m_username;

    uint64_t m_hipart;
    uint64_t m_lopart;
};
