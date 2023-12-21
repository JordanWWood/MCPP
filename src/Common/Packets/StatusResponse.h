#pragma once
#include "IPacket.h"
#include "Common/PacketPayload.h"
#include "Core/MCServer.h"

struct SStatusResponse : IPacket
{
    // We'll never want to serialize this packet. Only used for the server list
    virtual void Deserialize(char* start) override {}
    
    virtual SPacketPayload Serialize()
    {
        SPacketPayload payload;

        std::string string =
            R"({"version":{"name":"1.20.4","protocol":765},"players":{"max":10000,"online":0,"sample":[]},"description":{"text":"MCPP Server"},"favicon":"","enforceSecureChat":true,"previewsChat":true})";
        
        payload.m_payload = new char[string.size() + 3];
        
        payload.m_payload[0] = static_cast<char>(188);
        payload.m_payload[1] = 0;
        payload.m_payload[2] = static_cast<uint8_t>(string.size());
        memcpy(&payload.m_payload[3], string.c_str(), string.size());

        payload.m_size = string.size() + 3;
        
        return payload;
    }

    
};
