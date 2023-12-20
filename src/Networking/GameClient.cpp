#include "GameClient.h"

#include <cstdio>
#include <memory>
#include <string>

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include "Packets/BasePacket.h"

#define DEFAULT_BUFLEN 512

// TODO do something better than this
#define PROTOCOL_VERSION 765

struct SHandShake final : CBasePacket // TODO packets
{
    virtual char* Deserialize(char* start) override
    {
        uint32_t offset = 0;
        m_protocolVersion = DeserializeVarInt(start, offset);
        m_address = DeserializeString(&start[offset], 32, offset);
        m_port = DeserializeShort(&start[offset], offset);
        m_nextState = static_cast<uint8_t>(start[offset]);

        return &start[offset + 1];
    }
    
    uint32_t m_protocolVersion { 0 };
    std::string m_address;
    uint16_t m_port { 0 };
    uint8_t m_nextState { 0 };
};

CGameClient::~CGameClient()
{
    if(m_clientSocket != INVALID_SOCKET)
        closesocket(m_clientSocket);
}

bool CGameClient::RecvPackets()
{
    char recvBuffer[DEFAULT_BUFLEN];
    constexpr int recvBufferLength{ DEFAULT_BUFLEN };
    
    const int iResult = recv(m_clientSocket, recvBuffer, recvBufferLength, 0);
    if (iResult > 0)
    {
        char* end = nullptr;
        do
        {
            if (end != nullptr)
                ++end;
            
            uint8_t payloadSize = static_cast<uint8_t>(recvBuffer[0]);
            uint8_t packetId = static_cast<uint8_t>(recvBuffer[1]);
        
            if (m_clientState == EClientState::eCS_Handshake)
            {
                SHandShake handShake;
                end = handShake.Deserialize(&recvBuffer[2]);

                // TODO range of support
                if (handShake.m_protocolVersion != PROTOCOL_VERSION)
                {
                    closesocket(m_clientSocket);
                    m_socketState = ESocketState::eSS_CLOSED;
                    m_clientSocket = INVALID_SOCKET;
                    
                    return nullptr;
                }
            
                switch(handShake.m_nextState)
                {
                case 1: m_clientState = EClientState::eCS_Status; break;
                case 2: m_clientState = EClientState::eCS_Login; break;
                default:
                    // TODO something is wrong
                        break;
                }
            }
        } while (*end != 0);
        
        return true;
    }

    if (iResult == 0)
    {
        // We're done processing the queue.
        return nullptr;
    }

    const int error = WSAGetLastError();
    if(error == WSAEWOULDBLOCK)
    {
        // We're just waiting for something to receive. Break and we'll check if theres something next time
        return nullptr;
    }

    // If we make it here something went wrong
    printf("recv failed with error: %d\n", WSAGetLastError());
    
    closesocket(m_clientSocket);
    m_socketState = ESocketState::eSS_CLOSED;
    
    return nullptr;
}
