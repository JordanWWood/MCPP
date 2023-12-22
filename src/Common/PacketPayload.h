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

    char* GetDeserializeStartPtr() { return m_payload + m_startOffset; }

    char* m_payload{ nullptr };
    
    uint32_t m_packetId{ 0 };
    uint32_t m_size{ 0 };
    uint32_t m_startOffset{ 0 };
};

inline SPacketPayload::~SPacketPayload()
{
    delete[] m_payload;
    m_payload = nullptr;
}
