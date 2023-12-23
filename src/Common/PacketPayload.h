#pragma once
#include <cstdint>

struct SPacketPayload
{
    SPacketPayload() = default;
    
    ~SPacketPayload();
    SPacketPayload(SPacketPayload&& other) noexcept;
    SPacketPayload(const SPacketPayload& other);

    SPacketPayload& operator=(SPacketPayload&& other);
    SPacketPayload& operator=(const SPacketPayload& other);

    char* GetDeserializeStartPtr() const { return m_payload + m_startOffset; }

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

inline SPacketPayload::SPacketPayload(SPacketPayload&& other) noexcept
{
    m_payload = other.m_payload;
    m_size = other.m_size;
    m_packetId = other.m_packetId;

    other.m_payload = nullptr;
}

inline SPacketPayload::SPacketPayload(const SPacketPayload& other)
{
    m_packetId = other.m_packetId;
    m_size = other.m_size;
    m_startOffset = other.m_startOffset;
        
    m_payload = new char[other.m_size];
    memcpy(m_payload, other.m_payload, other.m_size);
}

inline SPacketPayload& SPacketPayload::operator=(SPacketPayload&& other)
{
    m_payload = other.m_payload;
    m_size = other.m_size;
    m_packetId = other.m_packetId;

    other.m_payload = nullptr;

    return *this;
}

inline SPacketPayload& SPacketPayload::operator=(const SPacketPayload& other)
{
    m_packetId = other.m_packetId;
    m_size = other.m_size;
    m_startOffset = other.m_startOffset;
        
    m_payload = new char[other.m_size];
    memcpy(m_payload, other.m_payload, other.m_size);
    return *this;
}
