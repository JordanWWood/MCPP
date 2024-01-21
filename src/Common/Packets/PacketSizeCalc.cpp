#include "PacketSizeCalc.h"

void CPacketSizeCalc::OnVarInt(int& value)
{
    if(!m_success)
        return;
    
    uint32_t size = 0;
    int finalValue = value;
    if(m_isEnd)
        finalValue = m_currentSize;
    
    while (true) {
        size++;
        
        if ((finalValue & ~SEGMENT_BITS) == 0) {
            m_currentSize += size;
            if(m_isEnd)
                m_packetLengthSize = size;
            
            return;
        }

        finalValue = finalValue >> 7;
    }
}

void CPacketSizeCalc::OnULong(uint64_t& /* value */)
{
    if(!m_success)
        return;
    
    constexpr int size = sizeof(uint64_t);
    m_currentSize += size;
}

void CPacketSizeCalc::OnString(std::string& value, const uint32_t maxSize)
{
    if(!m_success)
        return;

    int lengthSize = value.size();
    OnVarInt(lengthSize);
    
    if(lengthSize > maxSize)
    {
        m_success = false;
        return;
    }

    m_currentSize += value.length();
}

void CPacketSizeCalc::OnUInt8(uint8_t& value)
{
    if(!m_success)
        return;

    constexpr int size = sizeof(uint8_t);
    m_currentSize += size;
}

void CPacketSizeCalc::OnUUID(CUUID& uuid)
{
    m_currentSize += sizeof(uint64_t) * 2;
}

void CPacketSizeCalc::OnShort(uint16_t& value)
{
    if(!m_success)
        return;

    constexpr int size = sizeof(uint16_t);
    m_currentSize += size;
}
