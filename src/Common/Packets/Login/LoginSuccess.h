#pragma once

#include "Packets/IPacket.h"
#include "uuid.h"

struct SLoginSuccess : public IPacket
{
    SLoginSuccess() : IPacket(0x02) {}
    
    // TODO this will not work for deserialisation and is quite frankly awful the macro API needs to be built out more
    SERIALIZE_BEGIN()
        const std::string uuidBytes = m_id.bytes();
        SERIALIZE_ULONG(*(uint64_t*)uuidBytes.c_str())
        SERIALIZE_ULONG(*(uint64_t*)(uuidBytes.c_str() + 8))
        SERIALIZE_STRING(m_username, 16)
        uint32_t length = m_properties.size();
        SERIALIZE_VARINT(length)
        for(SProperty& prop : m_properties)
        {
            SERIALIZE_STRING(prop.m_name, 32767)
            SERIALIZE_STRING(prop.m_value, 32767)

            if(prop.m_signed)
                SERIALIZE_U8(*reinterpret_cast<uint8_t*>(&prop.m_signed))
            
            SERIALIZE_STRING(prop.m_signature, 32767)
        }
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
