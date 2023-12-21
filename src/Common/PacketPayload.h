#pragma once
#include <cstdint>

struct SPacketPayload
{
    SPacketPayload() = default;
    
    ~SPacketPayload();
    SPacketPayload(SPacketPayload&& other) noexcept
    {
        m_payload = other.m_payload;
        m_size = other.m_size;
        m_packetId = other.m_packetId;

        other.m_payload = nullptr;
    }
    
    uint8_t m_packetId{ 0 };
    
    char* m_payload{ nullptr };
    uint8_t m_size{ 0 };
};

inline SPacketPayload::~SPacketPayload()
{
    delete[] m_payload;
    m_payload = nullptr;
}
