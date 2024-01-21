#include "PacketReader.h"

#include "Platform.h"

void CPacketReader::OnShort(uint16_t& value)
{
    const uint16_t reverseInt = *reinterpret_cast<uint16_t*>(m_current);

#ifdef LITTLEENDIAN
    // packets are encoded big endian. So we need to invert the result if the system is little endian
    value = forceswap16(reverseInt);
#else
    value = reverseInt;
#endif
    
    m_current += sizeof(uint16_t);
}

void CPacketReader::OnVarInt(int& value)
{
    int position = 0;

    while (true) {
        uint8_t byte = static_cast<uint8_t>(*m_current);
        ++m_current;
            
        value |= (byte & SEGMENT_BITS) << position;

        if((byte & CONTINUE_BIT) == 0) break;

        position += 7;

        if (position >= 32) {
            value = 0;
            return;
        }
    }
}

void CPacketReader::OnULong(uint64_t& value)
{
    value = *reinterpret_cast<uint64_t*>(m_current);
    m_current += sizeof(uint64_t);
}

void CPacketReader::OnString(std::string& value, uint32_t maxSize)
{
    if(maxSize >= MAX_STRING_LENGTH)
        return;

    int length = 0;
    OnVarInt(length);

    // Adding the offset since we want to trim the size of the string
    value = std::string(m_current, length);
    m_current += length;
}

void CPacketReader::OnUInt8(uint8_t& value)
{
    value = static_cast<uint8_t>(*m_current);
    m_current += sizeof(uint8_t);
}

void CPacketReader::OnUUID(CUUID& uuid)
{
    uint64_t y = 0;
    OnULong(y);

    uint64_t x = 0;
    OnULong(x);

    uuid = CUUID(x, y);
}
