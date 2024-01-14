#pragma once

#include "Packets/IPacket.h"
#include "PacketPayload.h"
#include "uuid.h"

struct SLoginSuccess : public IPacket
{
    // TODO for connecting to servers
    virtual void Deserialize(char* start) override {}

    virtual SPacketPayload Serialize() override
    {
        OPTICK_EVENT();
        
        const uint32_t packetIdSize = VarIntSize(0x02);
        const uint32_t idSize = 16;
        const uint32_t usernameLengthSize = VarIntSize(m_username.size());
        const uint32_t usernameLength = m_username.size();
        const uint32_t numPropertiesSize = VarIntSize(m_properties.size());

        uint32_t totalPropertySize = 0;
        for (const SProperty& prop : m_properties)
        {
            totalPropertySize += VarIntSize(prop.m_name.size());
            totalPropertySize += prop.m_name.size();

            totalPropertySize += VarIntSize(prop.m_value.size());
            totalPropertySize += prop.m_value.size();

            totalPropertySize += 1;

            totalPropertySize += VarIntSize(prop.m_signature.size());
            totalPropertySize += prop.m_signature.size();
        }
        
        const uint32_t packetLength = packetIdSize + idSize + usernameLengthSize + usernameLength + numPropertiesSize + totalPropertySize;
        const uint32_t packetLengthSize = VarIntSize(packetLength);
        
        SPacketPayload payload;
        payload.m_payload = new char[packetLength + packetLengthSize];

        uint32_t offset = 0;
        SerializeVarInt(payload.m_payload, packetLength, offset);
        SerializeVarInt(payload.m_payload + offset, 0x02, offset);
        
        std::string uuidBytes = m_id.bytes();
        SerializeULong(payload.m_payload + offset, *(uint64_t*)uuidBytes.c_str(), offset);
        SerializeULong(payload.m_payload + offset, *(uint64_t*)(uuidBytes.c_str() + 8), offset);
        
        SerializeVarInt(payload.m_payload + offset, m_username.size(), offset);
        memcpy(payload.m_payload + offset, m_username.c_str(), m_username.size());
        offset += m_username.size();

        SerializeVarInt(payload.m_payload + offset, m_properties.size(), offset);

        for(const SProperty& prop : m_properties)
        {
            SerializeVarInt(payload.m_payload + offset, prop.m_name.size(), offset);
            memcpy(payload.m_payload + offset, prop.m_name.c_str(), prop.m_name.size());
            offset += prop.m_name.size();
            
            SerializeVarInt(payload.m_payload + offset, prop.m_value.size(), offset);
            memcpy(payload.m_payload + offset, prop.m_value.c_str(), prop.m_value.size());
            offset += prop.m_value.size();
            
            memset(payload.m_payload + offset, prop.m_signed, 1);
            offset += 1;

            SerializeVarInt(payload.m_payload + offset, prop.m_signature.size(), offset);
            memcpy(payload.m_payload + offset, prop.m_signature.c_str(), prop.m_signature.size());
            offset += prop.m_signature.size();
        }
        
        payload.m_size = offset;

        if (payload.m_size != (packetLength + packetLengthSize))
            MCLog::critical("Mismatched payload size. This can lead to memory corruption! FinalSize[{}] ExpectedSize[{}]", payload.m_size, packetLength + packetLengthSize);
        
        return payload;
    }

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
