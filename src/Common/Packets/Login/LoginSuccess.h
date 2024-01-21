#pragma once

#include "Packets/IPacket.h"
#include "Uuid.h"

struct SLoginSuccess : public IPacket
{
    SLoginSuccess() : IPacket(0x02) {}

    SERIALIZE_BEGIN()
        SERIALIZE_UUID(m_id)
        SERIALIZE_STRING(m_username, 16)
        int i = 0;
        SERIALIZE_VARINT(i)
        // SERIALIZE_ARRAY_BEGIN(m_properties, SProperty)
        //     SERIALIZE_STRING(current.m_name, MAX_STRING_LENGTH)
        //     SERIALIZE_STRING(current.m_value, MAX_STRING_LENGTH)
        //     SERIALIZE_U8(*reinterpret_cast<uint8_t*>(&current.m_signed))
        //     SERIALIZE_STRING(current.m_signature, MAX_STRING_LENGTH)
        // SERIALIZE_ARRAY_END(m_properties)
    SERIALIZE_END()

    struct SProperty
    {
        std::string m_name;
        std::string m_value;
        bool m_signed{ false };
        std::string m_signature;
    };

    CUUID m_id;
    std::string m_username;
    std::vector<SProperty> m_properties;
};
