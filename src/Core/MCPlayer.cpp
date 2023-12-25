#include "pch.h"
#include "MCPlayer.h"

#include <random>

#include "Common/Packets/Handshake.h"
#include "Common/Packets/IPacket.h"
#include "Common/PacketPayload.h"
#include "Common/Encryption/IRSAKeyPair.h"
#include "Common/Packets/EncryptionRequest.h"
#include "Common/Packets/EncryptionResponse.h"
#include "Common/Packets/LoginStart.h"
#include "Common/Packets/StatusResponse.h"

#include "Common/IConnection.h"

#include "Common/Utils/uuid.h"

#include "curl_easy.h"
#include "curl_ios.h"
#include "curl_exception.h"

// TODO do something better than this
#define PROTOCOL_VERSION 765

void CMCPlayer::NetworkTick()
{
    OPTICK_EVENT();

    bool success = m_pConnection->RecvPackets(this);
    if(!success)
        MCLog::warn("Failure to recv packets needs to be implemented");

    // Update any running get request
    for(CHTTPGet& request : m_runningGetRequest)
        request.Update();
}

bool CMCPlayer::ProcessPacket(SPacketPayload&& payload)
{
    OPTICK_EVENT();

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

bool CMCPlayer::IsDead() const
{
    return m_pConnection->IsSocketClosed();
}

bool CMCPlayer::HandleHandshake(SPacketPayload&& payload)
{
    OPTICK_EVENT();

    SHandshake handshake;
    handshake.Deserialize(payload.GetDeserializeStartPtr());

    if (handshake.m_protocolVersion != PROTOCOL_VERSION)
        return false;
            
    switch(handshake.m_nextState)
    {
    case 1: m_state = EClientState::eCS_Status; break;
    case 2: m_state = EClientState::eCS_Login; break;
    default:
        MCLog::error("Incorrect next state in handshake. NextState[{}]", handshake.m_nextState);
        return false;
    }

    return true;
}

bool CMCPlayer::HandleLogin(SPacketPayload&& payload)
{
    OPTICK_EVENT();

    if(payload.m_packetId == 0)
    {
        MCLog::debug("Received login start. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        
        LoginStart loginStart;
        loginStart.Deserialize(payload.GetDeserializeStartPtr());

        m_username = loginStart.m_username;
        
        MCLog::debug("User from {} is {}", m_pConnection->GetRemoteAddress(), GetUsername());
        SendEncryptionRequest();

        MCLog::debug("Sent encryption request. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        
        return true;
    }

    if (payload.m_packetId == 1)
    {
        MCLog::debug("Received encryption response. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());

        SEncryptionResponse response;
        response.m_pServerKey = m_pServerKey;

        response.Deserialize(payload.GetDeserializeStartPtr());
        MCLog::debug("Deserialized encryption response. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        
        if (response.m_verifyTokenValue != m_verifyToken)
            return false;
        
        m_pConnection->EnableEncryption();
        m_pConnection->SetAESKey(response.m_sharedSecret);
        
        MCLog::debug("Enabled encryption. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        MCLog::debug("Beginning authentication with mojang. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());

        std::string digest = m_pConnection->GenerateHexDigest(m_pServerKey->GetAsnDerKey(), response.m_sharedSecret);
        MCLog::debug("Generated digest. Digest[{}] Address[{}] Username[{}]", digest, m_pConnection->GetRemoteAddress(), GetUsername());        

        std::string url("https://sessionserver.mojang.com/session/minecraft/hasJoined?username=");
        url.append(GetUsername());
        url.append("&serverId=");
        url.append(digest);
        
        // TODO it'd be good to have something that explicitly handles curl requests along with updating them. This is nasty but I want to keep powering on with the login flow
        CHTTPGet& request = m_runningGetRequest.emplace_back(CHTTPGet());
        request.AddRequest(url, [](bool success, std::string body) {

        });

        MCLog::debug("Queued authentication request. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        
        return true;
    }

    return true;
}

bool CMCPlayer::HandleStatus(SPacketPayload&& payload)
{
    OPTICK_EVENT();

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
    OPTICK_EVENT();

    SEncryptionRequest request;
    std::string publicKey = m_pServerKey->GetAsnDerKey();
        
    request.m_publicKeyLength = publicKey.size();
    request.m_publicKey = std::move(publicKey);

    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    for (std::size_t i = 0; i < 64; ++i)
        m_verifyToken += CHARACTERS[distribution(generator)];

    request.m_verifyTokenLength = m_verifyToken.size();
    request.m_verifyToken = m_verifyToken;
    
    m_pConnection->SendPacket(request.Serialize());

    return true;
}
