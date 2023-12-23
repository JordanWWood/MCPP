#include "MCPlayer.h"

#include <spdlog/spdlog.h>

#include <random>

#include "Common/Packets/Handshake.h"
#include "Common/Packets/IPacket.h"
#include "Common/PacketPayload.h"
#include "Common/Encryption/IRSAKeyPair.h"
#include "Common/Packets/EncryptionRequest.h"
#include "Common/Packets/LoginStart.h"
#include "Common/Packets/StatusResponse.h"

// TODO do something better than this
#define PROTOCOL_VERSION 765

void CMCPlayer::RecvPackets()
{
    bool success = m_pConnection->RecvPackets(this);
    if(!success)
        spdlog::warn("Failure to recv packets needs to be implemented");
    // TODO handle errors
}

bool CMCPlayer::ProcessPacket(SPacketPayload&& payload)
{
    switch (m_state)
    {
    case EClientState::eCS_Handshake:
        return HandleHandshake(std::move(payload));
        
    case EClientState::eCS_Login:
        return HandleLogin(std::move(payload));
        
    case EClientState::eCS_Status:
        return HandleStatus(std::move(payload));

    case EClientState::eCS_Configuration:
        // TODO
        break;
    case EClientState::eCS_Play:
        // TODO
        break;
    }

    return true;
}

bool CMCPlayer::HandleHandshake(SPacketPayload&& payload)
{
    SHandshake handshake;
    handshake.Deserialize(payload.GetDeserializeStartPtr());

    if (handshake.m_protocolVersion != PROTOCOL_VERSION)
        return false;
            
    switch(handshake.m_nextState)
    {
    case 1: m_state = EClientState::eCS_Status; break;
    case 2: m_state = EClientState::eCS_Login; break;
    default:
        spdlog::error("Incorrect next state in handshake. NextState[{}]", handshake.m_nextState);
        return false;
    }

    return true;
}

bool CMCPlayer::HandleLogin(SPacketPayload&& payload)
{
    if(payload.m_packetId == 0)
    {
        spdlog::debug("Received login start. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        
        LoginStart loginStart;
        loginStart.Deserialize(payload.GetDeserializeStartPtr());

        m_username = loginStart.m_username;
        
        spdlog::debug("User from {} is {}", m_pConnection->GetRemoteAddress(), GetUsername());
        
        // TODO mojang auth
        
        // Send over our public key
        SendEncryptionRequest();
        
        return true;
    }

    if (payload.m_packetId == 1)
    {
        spdlog::debug("Received encryption response. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
    }

    return true;
}

bool CMCPlayer::HandleStatus(SPacketPayload&& payload)
{
    if(payload.m_packetId == 0)
    {
        SStatusResponse response;
        m_pConnection->SendPacket(response.Serialize());
    }

    if(payload.m_packetId == 1)
    {
        payload.m_size = payload.m_size + 2;
        m_pConnection->SendPacket(std::move(payload));
    }
    return true;
}

bool CMCPlayer::SendEncryptionRequest()
{
    SEncryptionRequest request;
    std::string publicKey = m_pServerKey->GetAsnDerKey();
        
    request.m_publicKeyLength = publicKey.size();
    request.m_publicKey = std::move(publicKey);

    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    for (std::size_t i = 0; i < 64; ++i)
        random_string += CHARACTERS[distribution(generator)];

    request.m_verifyTokenLength = random_string.size();
    request.m_verifyToken = std::move(random_string);
    
    m_pConnection->SendPacket(request.Serialize());

    return true;
}
