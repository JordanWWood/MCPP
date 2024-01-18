#include "PacketWriter.h"

void CPacketWriter::OnShort(uint16_t& value)
{
    // TODO
}

void CPacketWriter::OnVarInt(uint32_t& value)
{
    const uint32_t startOffset = m_size;
    while (true) {
        const uint32_t position = m_size - startOffset;
        m_size++;
        
        if ((value & ~SEGMENT_BITS) == 0) {
            m_data[position] = static_cast<char>(value);
            return;
        }

        m_data[position] = static_cast<char>((value & SEGMENT_BITS) | CONTINUE_BIT);
        value = value >> 7;
    }
}

void CPacketWriter::OnULong(uint64_t& value)
{
    constexpr int size = sizeof(uint64_t);
    memcpy(m_data + m_size, &value, size);
    m_size += size;
}

void CPacketWriter::OnString(std::string& value, const uint32_t maxSize)
{
    if (value.length() > maxSize)
        return;

    uint32_t size = value.size();
    OnVarInt(size);
    memcpy(m_data + m_size, value.c_str(), value.size());
    m_size += value.size();
}

void CPacketWriter::OnUInt8(uint8_t& value)
{
}

void CPacketWriter::OnUUID(CUUID uuid)
{
    
}
