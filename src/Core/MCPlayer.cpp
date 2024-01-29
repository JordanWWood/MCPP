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
#include "PacketUtils.h"
#include "Packets/PacketReader.h"
#include "Packets/Login/LoginSuccess.h"

// TODO do something better than this
#define PROTOCOL_VERSION 765

void CMCPlayer::NetworkTick()
{
    MCPP_PROFILE_SCOPE()

    bool success = m_pConnection->RecvPackets(this);
    if (!success)
        MCLog::warn("Failure to recv packets needs to be implemented");
}

bool CMCPlayer::ProcessPacket(SPacketPayload&& payload)
{
    MCPP_PROFILE_SCOPE()

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

const std::string& CMCPlayer::GetRemoteAddress() const
{
    return m_pConnection->GetRemoteAddress();
}

bool CMCPlayer::HandleHandshake(SPacketPayload&& payload)
{
    MCPP_PROFILE_SCOPE()

    SHandshake handshake;

    CPacketReader reader(payload.m_payload);
    handshake.Serialize(reader);

    if (handshake.m_protocolVersion != PROTOCOL_VERSION)
        return false;

    switch (handshake.m_nextState)
    {
    case 1: m_state = EClientState::eCS_Status;
        break;
    case 2: m_state = EClientState::eCS_Login;
        break;
    default:
        MCLog::error("Incorrect next state in handshake. NextState[{}]", handshake.m_nextState);
        return false;
    }

    return true;
}

bool CMCPlayer::HandleLogin(SPacketPayload&& payload)
{
    MCPP_PROFILE_SCOPE()

    if (payload.m_packetId == 0)
    {
        MCLog::debug("Received login start. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(),
                     GetUsername());

        SLoginStart loginStart;
        CPacketReader reader(payload.m_payload);
        loginStart.Serialize(reader);

        m_username = loginStart.m_username;
        m_uuid = loginStart.m_uuid;

        MCLog::debug("User from {} is {}", m_pConnection->GetRemoteAddress(), GetUsername());

        // If we're online we want to encrypt the connection
        if (IGlobalEnvironment::Get()->IsOnline())
        {
            SendEncryptionRequest();
            MCLog::debug("Sent encryption request. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(),
                         GetUsername());
            return true;
        }

        SLoginSuccess loginSuccess;
        loginSuccess.m_id = m_uuid;
        loginSuccess.m_username = m_username;

        m_pConnection->QueuePacket(SerializePacket(loginSuccess));
        return true;
    }

    if (payload.m_packetId == 1)
    {
        MCLog::debug("Received encryption response. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(),
                     GetUsername());

        const std::shared_ptr<IRSAKeyPair> pKey = IGlobalEnvironment::Get()->GetNetwork().lock()->GetServerKeyPair();

        SEncryptionResponse response;
        response.m_pServerKey = pKey;

        CPacketReader reader(payload.m_payload);
        response.Serialize(reader);

        MCLog::debug("Deserialized encryption response. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(),
                     GetUsername());

        if (response.m_verifyTokenValue != m_verifyToken)
            return false;

        m_pConnection->EnableEncryption();
        m_pConnection->SetAESKey(response.m_sharedSecret);

        MCLog::debug("Enabled encryption. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(), GetUsername());
        MCLog::debug("Beginning authentication with mojang. Address[{}] Username[{}]",
                     m_pConnection->GetRemoteAddress(), GetUsername());

        std::string digest = IGlobalEnvironment::Get()->GetNetwork().lock()->GenerateHexDigest(
            pKey->GetAsnDerKey(), response.m_sharedSecret);
        MCLog::debug("Generated digest. Digest[{}] Address[{}] Username[{}]", digest, m_pConnection->GetRemoteAddress(),
                     GetUsername());

        std::string url("https://sessionserver.mojang.com/session/minecraft/hasJoined?username=");
        url.append(GetUsername());
        url.append("&serverId=");
        url.append(digest);

        // TODO send over the ip if we want to prevent proxy connections

        IGlobalEnvironment::Get()->GetCurl().lock()->QueueHttpGet(
            url, [this, username = GetUsername()](bool success, std::string body)
            {
                MCPP_PROFILE_SCOPE()

                nlohmann::json jsonBody = nlohmann::json::parse(body);  

                std::string id;
                jsonBody["id"].get_to(id);

                SLoginSuccess request;
                request.m_id = CUUID::fromStrFactory(ConvertSlimToFullUUID(id));
                request.m_username = username;

                nlohmann::json props = jsonBody["properties"];
                if (props.is_array())
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

                MCLog::debug("Sending LoginSuccess. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(),
                             GetUsername());
                m_pConnection->QueuePacket(SerializePacket(request));
            });

        MCLog::debug("Queued authentication request. Address[{}] Username[{}]", m_pConnection->GetRemoteAddress(),
                     GetUsername());

        return true;
    }

    if (payload.m_packetId == 3)
    {
        MCLog::debug("Received login ack. Username[{}] UUID[{}]", m_username, m_uuid.str());
        m_state = EClientState::eCS_Configuration;

        // TEMP return false to disconnect the client
        return true;
    }

    return true;
}

bool CMCPlayer::HandleStatus(SPacketPayload&& payload)
{
    MCPP_PROFILE_SCOPE()

    if (payload.m_packetId == 0)
    {
        SStatusResponse response;

        // TODO replace this with some actual stats
        response.m_body = {
            {
                "version", {
                    {"name", "1.20.4"},
                    {"protocol", 765}
                }
            },
            {
                "players", {
                    {"max", 10000},
                    {"online", 0}
                }
            },
            {
                "description", {
                    {"text", "MCPP Server"}
                }
            },
            {"enforceSecureChat", true},
            {"previewsChat", true}
        };

        m_pConnection->QueuePacket(SerializePacket(response));
    }

    if (payload.m_packetId == 1)
    {
        payload.m_size = payload.m_size + 2;
        m_pConnection->QueuePacket(std::move(payload));
    }
    return true;
}

bool CMCPlayer::SendEncryptionRequest()
{
    MCPP_PROFILE_SCOPE()

    SEncryptionRequest request;
    std::string publicKey = IGlobalEnvironment::Get()->GetNetwork().lock()->GetServerKeyPair()->GetAsnDerKey();

    request.m_publicKeyLength = static_cast<uint32_t>(publicKey.size());
    request.m_publicKey = std::move(publicKey);

    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, static_cast<int>(CHARACTERS.size()) - 1);

    for (std::size_t i = 0; i < 64; ++i)
        m_verifyToken += CHARACTERS[distribution(generator)];

    request.m_verifyTokenLength = static_cast<uint32_t>(m_verifyToken.size());
    request.m_verifyToken = m_verifyToken;

    m_pConnection->QueuePacket(SerializePacket(request));

    return true;
}
