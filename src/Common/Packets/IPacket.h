#pragma once

#include <cstdint>
#include <string>

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

class IPacket
{
public:
    virtual ~IPacket() = default;
    virtual void Deserialize(char* start) = 0;
    virtual SPacketPayload Serialize() = 0;
    
protected:
    static uint16_t DeserializeShort(char* start, uint32_t& offset)
    {
        offset += 2;

        const uint16_t reverseInt = *reinterpret_cast<uint16_t*>(start);
        return (reverseInt << 8) | ((reverseInt >> 8) & 0x00ff);
    }
    
    static uint32_t DeserializeVarInt(char* start, uint32_t& end)
    {
        uint32_t value = 0;
        int position = 0;
        int bytes = 0;
        
        while (true) {
            uint8_t byte = static_cast<uint8_t>(*start);
            ++start;

            ++bytes;
            value |= (byte & SEGMENT_BITS) << position;

            if((byte & CONTINUE_BIT) == 0) break;

            position += 7;

            if (position >= 32) return 0;
        }
        end = bytes;
        return value;
    }

    static std::string DeserializeString(char* start, uint32_t maxSize, uint32_t& offset)
    {
        if(maxSize >= MAX_STRING_LENGTH)
            return "";
        
        uint32_t length = static_cast<uint32_t>(*start);
        offset = offset + length + 1;

        // Plus one since we want to trim the string length
        std::string string(start + 1, length);
        
        return string;
    }
};
