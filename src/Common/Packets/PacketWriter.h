#pragma once
#include "IPacket.h"

class CPacketWriter : IPacketVisitor
{
public:
    CPacketWriter(uint32_t size) : m_data(new char[size])
    {
        memset(m_data, 0, size);
    }

    bool SizeFirst() const override { return false; }
private:
    void OnShort(uint16_t& value) override;
    void OnVarInt(uint32_t& value) override;
    void OnULong(uint64_t& value) override;
    void OnString(std::string& value, const uint32_t maxSize) override;
    void OnUInt8(uint8_t& value) override;
    void OnUUID(CUUID& uuid) override;

private:
    char* m_data;
    uint32_t m_size{ 0 };
};
