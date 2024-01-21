#pragma once
#include <cstdint>

#include "Packets/IPacket.h"
#include "PacketPayload.h"

struct SHandshake final : public IPacket
{
    SHandshake() : IPacket(0x00) {}

    SERIALIZE_BEGIN()
        SERIALIZE_VARINT(m_protocolVersion)
        SERIALIZE_STRING(m_address, 32)
        SERIALIZE_SHORT(m_port)
        SERIALIZE_U8(m_nextState)
    SERIALIZE_END()

    int m_protocolVersion { 0 };
    std::string m_address;
    uint16_t m_port { 0 };
    uint8_t m_nextState { 0 };
};
