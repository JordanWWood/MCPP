#include "MCPlayer.h"

#include <spdlog/spdlog.h>

#include "Common/Packets/Handshake.h"
#include "Common/Packets/IPacket.h"
#include "Common/PacketPayload.h"
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
        LoginStart login_start;
        login_start.Deserialize(payload.GetDeserializeStartPtr());

        m_username = login_start.m_username;
        
        return true;
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
