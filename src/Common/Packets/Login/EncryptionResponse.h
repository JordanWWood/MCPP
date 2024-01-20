#pragma once

#include "Packets/IPacket.h"
#include "PacketPayload.h"
#include "Encryption/IRSAKeyPair.h"
#include "spdlog/fmt/bin_to_hex.h"

struct SEncryptionResponse : public IPacket
{
    SEncryptionResponse() : IPacket(0x01) {}

    // We will never want to send this packet so this should be fine
    SERIALIZE_BEGIN()
    SERIALIZE_STRING(m_sharedSecret, MAX_STRING_LENGTH)
    m_sharedSecret = m_pServerKey->Decrypt(m_sharedSecret);
    SERIALIZE_STRING(m_verifyTokenValue, MAX_STRING_LENGTH)
    m_verifyTokenValue = m_pServerKey->Decrypt(m_verifyTokenValue);
    SERIALIZE_END()

    std::shared_ptr<IRSAKeyPair> m_pServerKey { nullptr };
    
    std::string m_sharedSecret;
    std::string m_verifyTokenValue;
};
