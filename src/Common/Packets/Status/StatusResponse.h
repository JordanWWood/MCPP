#pragma once
#include "Packets/IPacket.h"
#include "PacketPayload.h"

#include "nlohmann/json.hpp"

struct SStatusResponse : public IPacket
{
    // We'll never want to deserialize this packet. Only used for the server list
    virtual void Deserialize(char* start) override {}
    virtual SPacketPayload Serialize() override
    {
        {
            SPacketPayload payload;

            // TODO this needs to be created on the go
            nlohmann::json j = {
                {
                    "version", {
                        {"name", "1.20.4"},
                        {"protocol", 765}
                    }
                },
                {
                    "players", {
                        { "max", 10000 },
                        { "online", 0 }
                    }
                },
                {
                    "description", {
                        { "text", "MCPP Server" }
                    }
                },
                {"enforceSecureChat", true},
                {"previewsChat", true}
            };
            
            std::string string = nlohmann::to_string(j);
    
            const uint32_t stringLengthSize = VarIntSize(string.size());
            const uint32_t packetIdSize = VarIntSize(0);
            const uint32_t payloadSize = VarIntSize(packetIdSize + stringLengthSize + static_cast<uint32_t>(string.size()));
        
            payload.m_payload = new char[payloadSize + packetIdSize + stringLengthSize + string.size()];

            uint32_t offset = 0;
            SerializeVarInt(payload.m_payload, packetIdSize + stringLengthSize + static_cast<uint32_t>(string.size()), offset);
            SerializeVarInt(payload.m_payload + offset, 0, offset);
            SerializeVarInt(payload.m_payload + offset, string.size(), offset);
            memcpy(&payload.m_payload[offset], string.c_str(), string.size());

            payload.m_size = string.size() + offset;
        
            return payload;
        }
    }
};
