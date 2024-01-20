#include "PacketWriter.h"

void CPacketWriter::OnShort(uint16_t& value)
{
    // TODO
}

void CPacketWriter::OnVarInt(int& value)
{
    uint32_t valueCopy = value;
    while (true) {
        const uint32_t position = m_size;
        m_size++;
        
        if ((valueCopy & ~SEGMENT_BITS) == 0) {
            m_data[position] = static_cast<char>(valueCopy);
            return;
        }

        m_data[position] = static_cast<char>((valueCopy & SEGMENT_BITS) | CONTINUE_BIT);
        valueCopy = valueCopy >> 7;
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

void CPacketWriter::OnUUID(CUUID& uuid)
{
    
}
