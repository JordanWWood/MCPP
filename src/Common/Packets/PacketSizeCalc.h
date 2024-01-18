#pragma once
#include "IPacket.h"

class CPacketSizeCalc : IPacketVisitor
{
public:
    explicit CPacketSizeCalc() = default;

    bool Success() const { return m_success; }
    bool SizeFirst() const override { return true; }
private:
    void OnShort(uint16_t& value) override;
    void OnVarInt(uint32_t& value) override;
    void OnULong(uint64_t& value) override;
    void OnString(std::string& value, const uint32_t maxSize) override;
    void OnUInt8(uint8_t& value) override;
    
    uint32_t m_currentSize{ 0 };
    bool m_success{ true };
};
