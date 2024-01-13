#pragma once

#include "Packets/IPacket.h"
#include "PacketPayload.h"

struct SEncryptionRequest : public IPacket
{
    // We will connect to other servers unencrypted since it'll be offline mode
    // In theory we'll never need to Deserialize this packet
    virtual void Deserialize(char* start) override
    {
        
    }
    
    virtual SPacketPayload Serialize() override
    {
        OPTICK_EVENT();
        
        const uint8_t packetIdSize = VarIntSize(1);
        const uint32_t serverIdSize = VarIntSize(m_serverId.size());
        const uint8_t keyLengthSize = VarIntSize(m_publicKeyLength);
        const uint8_t verifyTokenLengthSize = VarIntSize(m_verifyTokenLength);
        
        const uint32_t payloadSize = keyLengthSize + packetIdSize + serverIdSize + verifyTokenLengthSize + static_cast<uint32_t>(m_verifyTokenLength + m_publicKeyLength + m_serverId.size());
        
        // We need to add the size of the prefix to the packet
        const int32_t totalSize = payloadSize + VarIntSize(payloadSize);
        
        SPacketPayload payload;
        payload.m_payload = new char[totalSize];
        memset(payload.m_payload, 0, totalSize);

        payload.m_size = totalSize;
        
        uint32_t offset = 0;
        SerializeVarInt(payload.m_payload, payloadSize, offset);
        SerializeVarInt(payload.m_payload + offset, 1, offset);
        
        SerializeVarInt(payload.m_payload + offset, m_serverId.size(), offset);
        // Serialize nothing since the server id should always be zero
        
        SerializeVarInt(payload.m_payload + offset, m_publicKey.size(), offset);
        memcpy(&payload.m_payload[offset], m_publicKey.c_str(), m_publicKeyLength);
        offset += m_publicKey.size();
        
        SerializeVarInt(payload.m_payload + offset, m_verifyTokenLength, offset);
        memcpy(&payload.m_payload[offset], m_verifyToken.c_str(), m_verifyTokenLength);
        offset += m_verifyToken.size();

        return payload;
    }

    std::string m_serverId;
    uint32_t m_publicKeyLength { 0 };
    std::string m_publicKey;
    uint32_t m_verifyTokenLength { 0 };
    std::string m_verifyToken;
};
