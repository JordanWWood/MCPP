#include "pch.h"
#include "MCPlayer.h"

#include <random>

#include "PacketPayload.h"
#include "Encryption/IRSAKeyPair.h"
#include "Packets/IPacket.h"
#include "Packets/Handshake/Handshake.h"
#include "Packets/Login/EncryptionRequest.h"
#include "Packets/Login/EncryptionResponse.h"
#include "Packets/Login/LoginStart.h"
#include "Packets/Status/StatusResponse.h"

#include "IConnection.h"
#include "IGlobalEnvironment.h"
#include "HTTP/HTTPGet.h"
#include "Packets/Login/LoginSuccess.h"

// TODO do something better than this
#define PROTOCOL_VERSION 765

void CMCPlayer::NetworkTick()
{
    OPTICK_EVENT();

    bool success = m_pConnection->RecvPackets(this);
    if(!success)
        MCLog::warn("Failure to recv packets needs to be implemented");

    // Update any running get request
    for(auto it = m_runningGetRequest.begin(); it != m_runningGetRequest.end();)
    {
        it->Update();
        if(it->IsComplete())
        {
            it = m_runningGetRequest.erase(it);
            continue;
        }

        ++it;
    }
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

        const std::shared_ptr<IRSAKeyPair> pKey = IGlobalEnvironment::Get()->GetNetwork()->GetServerKeyPair();
        
        SEncryptionResponse response;
        response.m_pServerKey = pKey;

        response.Deserialize(payload.GetDeserializeStartPtr());
        MCLog::debug("Deserialized encryption response. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        
        if (response.m_verifyTokenValue != m_verifyToken)
            return false;
        
        m_pConnection->EnableEncryption();
        m_pConnection->SetAESKey(response.m_sharedSecret);
        
        MCLog::debug("Enabled encryption. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        MCLog::debug("Beginning authentication with mojang. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());

        std::string digest = IGlobalEnvironment::Get()->GetNetwork()->GenerateHexDigest(pKey->GetAsnDerKey(), response.m_sharedSecret);
        MCLog::debug("Generated digest. Digest[{}] Address[{}] Username[{}]", digest, m_pConnection->GetRemoteAddress(), GetUsername());        

        std::string url("https://sessionserver.mojang.com/session/minecraft/hasJoined?username=");
        url.append(GetUsername());
        url.append("&serverId=");
        url.append(digest);

        // TODO send over the ip if we want to prevent proxy connections
        
        // TODO it'd be good to have something that explicitly handles curl requests along with updating them. This is nasty but I want to keep powering on with the login flow
        CHTTPGet& request = m_runningGetRequest.emplace_back();
        request.AddRequest(url, [this](bool success, std::string body) {
            nlohmann::json jsonBody = nlohmann::json::parse(body);

            std::string id;
            jsonBody["id"].get_to(id);

            SLoginSuccess request;
            request.m_id = UUIDv4::UUID::fromStrFactory(id);
            request.m_username = GetUsername();

            nlohmann::json props = jsonBody["properties"];
            if(props.is_array())
            {
                for (int i = 0; i < props.size(); i++)
                {
                    nlohmann::json prop = props[i];
                    SLoginSuccess::SProperty finalProp;
                    prop["name"].get_to(finalProp.m_name);
                    prop["value"].get_to(finalProp.m_value);
                    if (prop.contains("signature"))
                    {
                        finalProp.m_signed = true;
                        prop["signature"].get_to(finalProp.m_signature);
                    }

                    request.m_properties.push_back(finalProp);
                }
            }

            MCLog::debug("Sending LoginSuccess. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
            m_pConnection->SendPacket(request.Serialize());
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
    std::string publicKey = IGlobalEnvironment::Get()->GetNetwork()->GetServerKeyPair()->GetAsnDerKey();
        
    request.m_publicKeyLength = static_cast<uint32_t>(publicKey.size());
    request.m_publicKey = std::move(publicKey);

    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    const std::uniform_int_distribution<> distribution(0, static_cast<int>(CHARACTERS.size()) - 1);

    for (std::size_t i = 0; i < 64; ++i)
        m_verifyToken += CHARACTERS[distribution(generator)];

    request.m_verifyTokenLength = static_cast<uint32_t>(m_verifyToken.size());
    request.m_verifyToken = m_verifyToken;
    
    m_pConnection->SendPacket(request.Serialize());

    return true;
}
