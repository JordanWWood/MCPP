#pragma once
#include "Common/Packets/IPacket.h"
#include "Common/PacketPayload.h"

struct SLoginSuccess : public IPacket
{
    // TODO for connecting to servers
    virtual void Deserialize(char* start) override {}

    virtual SPacketPayload Serialize() override
    {
        const uint32_t packetIdSize = VarIntSize(2);
        const uint32_t idLengthSize = VarIntSize(m_id.size());
        const uint32_t idLength = m_id.size();
        const uint32_t usernameLengthSize = VarIntSize(m_username.size());
        const uint32_t usernameLength = m_username.size();
        const uint32_t numPropertiesSize = VarIntSize(m_numProperties);
        
        const uint32_t packetLength = packetIdSize + idLengthSize + idLength + usernameLengthSize + usernameLength + numPropertiesSize;
        const uint32_t packetLengthSize = VarIntSize(packetLength);
        
        SPacketPayload payload;
        payload.m_payload = new char[packetLength + packetLengthSize];

        uint32_t offset = 0;
        SerializeVarInt(payload.m_payload, packetLength, offset);
        SerializeVarInt(payload.m_payload + offset, 2, offset);

        SerializeVarInt(payload.m_payload + offset, m_id.size(), offset);
        memcpy(payload.m_payload + offset, m_id.c_str(), m_id.size());
        offset += m_id.size();

        SerializeVarInt(payload.m_payload + offset, m_username.size(), offset);
        memcpy(payload.m_payload + offset, m_username.c_str(), m_username.size());
        offset += m_username.size();

        SerializeVarInt(payload.m_payload + offset, m_numProperties, offset);

        payload.m_size = offset - 1;

        return payload;
    }

    std::string m_id;
    std::string m_username;
    int32_t m_numProperties{ 0 };
};
