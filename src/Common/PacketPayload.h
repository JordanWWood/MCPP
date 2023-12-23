#pragma once
#include <cstdint>

struct SPacketPayload
{
    SPacketPayload() = default;
    
    ~SPacketPayload();
    SPacketPayload(SPacketPayload&& other) noexcept
    {
        m_payload = std::move(other.m_payload);
        m_size = std::move(other.m_size);
        m_packetId = std::move(other.m_packetId);

        other.m_payload = nullptr;
    }

    SPacketPayload(const SPacketPayload&& other) = delete;

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
