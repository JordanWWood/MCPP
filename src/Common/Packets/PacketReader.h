#pragma once

#include "IPacket.h"

class CPacketReader : public IPacketVisitor
{
public:
    CPacketReader(char* start) : m_current(start) {}
    bool SizeFirst() const override { return true; }

    void OnShort(uint16_t& value) override;
    void OnVarInt(uint32_t& value) override;
    void OnULong(uint64_t& value) override;
    void OnString(std::string& value, const uint32_t maxSize) override;
    void OnUInt8(uint8_t& value) override;
    void OnUUID(CUUID& uuid) override;

private:
    char* m_current;
};
