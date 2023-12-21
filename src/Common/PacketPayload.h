#pragma once
#include <cstdint>

struct SPacketPayload
{
    ~SPacketPayload()
    {
        delete[] m_payload;
    }
    
    uint8_t m_packetId{ 0 };
    
    char* m_payload{ nullptr };
    uint8_t m_size{ 0 };
};
