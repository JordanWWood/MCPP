#pragma once
#include "IPacket.h"

class CPacketSizeCalc : public IPacketVisitor
{
public:
    explicit CPacketSizeCalc() = default;

    uint32_t GetFullSize() const { return m_currentSize; }
    uint32_t GetPayloadSize() const { return m_currentSize - m_packetLengthSize; }
    bool Success() const { return m_success; }
    bool SizeFirst() const override { return false; }

    void OnShort(uint16_t& value) override;
    void OnVarInt(int& value) override;
    void OnULong(uint64_t& value) override;
    void OnString(std::string& value, const uint32_t maxSize) override;
    void OnUInt8(uint8_t& value) override;
    void OnUUID(CUUID& uuid) override;

private:
    uint32_t m_currentSize{ 0 };
    uint32_t m_packetLengthSize{ 0 };
    
    bool m_success{ true };
};
