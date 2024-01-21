#pragma once

#include <cstdint>
#include <cstring>

struct SPacketPayload
{
    SPacketPayload() = default;
    
    ~SPacketPayload();
    SPacketPayload(SPacketPayload&& other) noexcept;
    SPacketPayload(const SPacketPayload& other);

    SPacketPayload& operator=(SPacketPayload&& other);
    SPacketPayload& operator=(const SPacketPayload& other);

    char* m_payload{ nullptr };
    
    int m_packetId{ 0 };
    int m_size{ 0 };
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
        
    m_payload = new char[other.m_size];
    memcpy(m_payload, other.m_payload, other.m_size);
    return *this;
}
