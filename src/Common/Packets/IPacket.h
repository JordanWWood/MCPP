#pragma once

#include <cstdint>
#include <string>

#include <endianness.h>

#define SEGMENT_BITS 0x7F
#define CONTINUE_BIT 0x80

#define MAX_STRING_LENGTH 32767

enum class EClientState : uint8_t
{
    eCS_Handshake,
    eCS_Status,
    eCS_Login,
    eCS_Configuration,
    eCS_Play
};

struct IPacket
{
    virtual ~IPacket() = default;
    virtual void Deserialize(char* start) = 0;
    virtual SPacketPayload Serialize() = 0;

    ////////////////////////////////////////////////
    // Serialization/Deserialization
    static uint16_t DeserializeShort(char* start, uint32_t& offset);

    static void SerializeULong(char* start, uint64_t value, uint32_t& offset);

    static int32_t DeserializeVarInt(char* start, uint32_t& offset);
    static void SerializeVarInt(char* start, int32_t value, uint32_t& offset);
    static uint8_t VarIntSize(int32_t value);

    static std::string DeserializeString(char* start, uint32_t maxSize, uint32_t& offset);
};

inline uint16_t IPacket::DeserializeShort(char* start, uint32_t& offset)
{
    offset += 2;
    
    const uint16_t reverseInt = *reinterpret_cast<uint16_t*>(start);
    
    // packets are encoded big endian. So we need to inert the result if the system is little endian
    return betole16(reverseInt);
}

inline void IPacket::SerializeULong(char* start, uint64_t value, uint32_t& offset)
{
    offset += 8;

    value = betole64(value);
    memcpy(start, &value, 8);
}

inline int32_t IPacket::DeserializeVarInt(char* start, uint32_t& offset)
{
    int32_t value = 0;
    int position = 0;

    while (true) {
        uint8_t byte = static_cast<uint8_t>(*start);
        ++start;
        ++offset;
            
        value |= (byte & SEGMENT_BITS) << position;

        if((byte & CONTINUE_BIT) == 0) break;

        position += 7;

        if (position >= 32) return 0;
    }
    
    return value;
}

inline void IPacket::SerializeVarInt(char* start, int32_t value, uint32_t& offset)
{
    const uint32_t startOffset = offset;
    while (true) {
        const uint32_t position = offset - startOffset;
        offset++;
        
        if ((value & ~SEGMENT_BITS) == 0) {
            start[position] = static_cast<char>(value);
            return;
        }

        start[position] = static_cast<char>((value & SEGMENT_BITS) | CONTINUE_BIT);
            
        value = value >> 7;
    }
}

inline uint8_t IPacket::VarIntSize(int32_t value)
{
    uint32_t size = 0;
    while (true) {
        size++;
        
        if ((value & ~SEGMENT_BITS) == 0) {
            return size;
        }
            
        value = value >> 7;
    }
}

inline std::string IPacket::DeserializeString(char* start, uint32_t maxSize, uint32_t& offset)
{
    if(maxSize >= MAX_STRING_LENGTH)
        return "";

    uint32_t startOffset = offset;
    const uint32_t length = DeserializeVarInt(start, offset);
    uint32_t endOffset = offset - startOffset;
    offset = offset + length;

    // Adding the offset since we want to trim the size of the string
    std::string string(start + endOffset, length);
        
    return string;
}
