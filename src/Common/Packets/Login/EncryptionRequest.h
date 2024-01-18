#pragma once

#include "Packets/IPacket.h"
#include "PacketPayload.h"

struct SEncryptionRequest : public IPacket
{
    SEncryptionRequest() : IPacket(0x01) {}

    SERIALIZE_BEGIN()
    SERIALIZE_STRING(m_serverId, 20)
    SERIALIZE_STRING(m_publicKey, 32767)
    SERIALIZE_STRING(m_verifyToken, 32767)
    SERIALIZE_END()

    std::string m_serverId;
    uint32_t m_publicKeyLength { 0 };
    std::string m_publicKey;
    uint32_t m_verifyTokenLength { 0 };
    std::string m_verifyToken;
};
