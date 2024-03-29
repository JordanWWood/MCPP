﻿#pragma once
#include "IPacket.h"

class CPacketWriter : public IPacketVisitor
{
public:
    CPacketWriter(uint32_t size);

    bool SizeFirst() const override { return true; }
    char* GetData() const { return m_data; }
private:
    void OnShort(uint16_t& value) override;
    void OnVarInt(int& value) override;
    void OnULong(uint64_t& value) override;
    void OnString(std::string& value, const uint32_t maxSize) override;
    void OnUInt8(uint8_t& value) override;
    void OnUUID(CUUID& uuid) override;

private:
    char* m_data;
    uint32_t m_size{ 0 };
};
