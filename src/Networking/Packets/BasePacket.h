#pragma once
#include <cstdint>
#include <string>

#define SEGMENT_BITS 0x7F
#define CONTINUE_BIT 0x80

#define MAX_STRING_LENGTH 32767

class BasePacket
{
protected:
    uint32_t DeserializeVarInt(char* start, uint32_t& end)
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

    std::string DeserializeString(char* start, uint32_t maxSize, uint32_t& offset)
    {
        if(maxSize >= MAX_STRING_LENGTH)
            return "";
        
        uint32_t length = static_cast<uint32_t>(*start);
        offset = length + 1;

        // Plus one since we want to trim the string length
        std::string string(start + 1, length);
        
        return string;
    }
};
