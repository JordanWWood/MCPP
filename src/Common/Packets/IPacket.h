#pragma once

#include <cstdint>
#include <string>

#include "Uuid.h"

enum
{
    SEGMENT_BITS = 0x7F,
    CONTINUE_BIT = 0x80
};

enum
{
    MAX_STRING_LENGTH = 32767
};

enum class EClientState : uint8_t
{
    eCS_Handshake,
    eCS_Status,
    eCS_Login,
    eCS_Configuration,
    eCS_Play
};

#define SERIALIZE_SHORT(short) visitor.OnShort(short);
#define SERIALIZE_VARINT(varInt) visitor.OnVarInt(varInt);
#define SERIALIZE_ULONG(ulong) visitor.OnULong(ulong);
#define SERIALIZE_STRING(string, maxSize) visitor.OnString(string, maxSize);
#define SERIALIZE_U8(u8) visitor.OnUInt8(u8);
#define SERIALIZE_UUID(uuid) visitor.OnUUID(uuid);
#define SERIALIZE_ARRAY_BEGIN(arr, type)           \
    int currentSize = (arr).size();                \
    SERIALIZE_VARINT(currentSize)                  \
    for(int i = 0; i < (currentSize); ++i)         \
    {                                              \
        type current = type();                     \
        if((arr).size() > 0)                       \
            current = (arr)[i];
        

#define SERIALIZE_ARRAY_END(arr)                   \
        if((arr).size() < 0)                       \
            (arr).push_back(current);              \
    }

#define SERIALIZE_BEGIN()                          \
void Serialize(IPacketVisitor& visitor) override { \
    if(visitor.SizeFirst())                        \
        SERIALIZE_VARINT(m_packetSize)             \
    SERIALIZE_VARINT(m_packetId)                        

#define SERIALIZE_END()                            \
   if(!visitor.SizeFirst()) {                      \
       visitor.m_isEnd = true;                     \
       SERIALIZE_VARINT(m_packetSize)              \
   }                                               \
}

struct IPacketVisitor
{
    virtual ~IPacketVisitor() = default;
    virtual bool SizeFirst() const = 0;
    
    virtual void OnShort(uint16_t& value) = 0;
    virtual void OnVarInt(int& value) = 0;
    virtual void OnULong(uint64_t& value) = 0;
    virtual void OnString(std::string& value, const uint32_t maxSize) = 0;
    virtual void OnUInt8(uint8_t& value) = 0;
    virtual void OnUUID(CUUID& uuid) = 0;

    bool m_isEnd = false;
};

struct IPacket
{
    IPacket() = delete;
    IPacket(uint32_t packetId) : m_packetId(packetId) {}
    virtual ~IPacket() = default;
    
    virtual void Serialize(IPacketVisitor& visitor) = 0;
    void SetSize(uint32_t size)
    {
        m_packetSize = size;
    }

    int m_packetId;
    int m_packetSize = 0;
};
