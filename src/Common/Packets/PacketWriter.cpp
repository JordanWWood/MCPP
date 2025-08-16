#include "PacketWriter.h"

#include <cstring>

CPacketWriter::CPacketWriter(uint32_t size): m_data(new char[size])
{
    std::memset(m_data, 0, size);
}

void CPacketWriter::OnShort(uint16_t& value)
{
    constexpr int size = sizeof(uint16_t);
#ifdef LITTLEENDIAN
    uint16_t swapped = forceswap16(value);
    std::memcpy(m_data + m_size, &swapped, size);
#else
    std::memcpy(m_data + m_size, &value, size);
#endif
    m_size += size;
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
    std::memcpy(m_data + m_size, &value, size);
    m_size += size;
}

void CPacketWriter::OnString(std::string& value, const uint32_t maxSize)
{
    if (value.length() > maxSize)
        return;

    int size = value.size();
    OnVarInt(size);
    memcpy(m_data + m_size, value.c_str(), value.size());
    m_size += value.size();
}

void CPacketWriter::OnUInt8(uint8_t& value)
{
    m_data[m_size] = static_cast<char>(value);
    m_size += sizeof(uint8_t);
}

void CPacketWriter::OnUUID(CUUID& uuid)
{
    const std::string uuidBytes = uuid.bytes();
    OnULong(*(uint64_t*)uuidBytes.c_str());
    OnULong(*(uint64_t*)(uuidBytes.c_str() + 8));
}
