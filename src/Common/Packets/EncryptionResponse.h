﻿#pragma once

#include "IPacket.h"
#include "Common/PacketPayload.h"
#include "Common/Encryption/IRSAKeyPair.h"

struct SEncryptionResponse : public IPacket
{
    virtual void Deserialize(char* start) override
    {
        uint32_t offset = 0;
        std::string encrptedSharedSecret = DeserializeString(start, 512, offset);
        m_sharedSecret = m_pServerKey->Decrypt(encrptedSharedSecret);

        std::string encryptedVerifyToken = DeserializeString(start + offset, 512, offset);
        m_verifyTokenValue = m_pServerKey->Decrypt(encryptedVerifyToken);
    }

    // TODO for connecting to other servers
    virtual SPacketPayload Serialize() override
    {
        return SPacketPayload();
    }

    std::shared_ptr<IRSAKeyPair> m_pServerKey { nullptr };
    std::string m_sharedSecret;
    std::string m_verifyTokenValue;
};