#include "pch.h"
#include "ClientConnection.h"

#include <vector>

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include "Common/IPacketHandler.h"
#include "Common/PacketPayload.h"
#include "Common/Packets/IPacket.h"

#define DEFAULT_BUFLEN 512

CClientConnection::~CClientConnection()
{
    if(m_clientSocket != INVALID_SOCKET)
        closesocket(m_clientSocket);
}

bool CClientConnection::RecvPackets(IPacketHandler* pHandler)
{
    char recvBuffer[DEFAULT_BUFLEN];
    constexpr int recvBufferLength{ DEFAULT_BUFLEN };

    memset(&recvBuffer, 0, recvBufferLength);
    
    const int iResult = recv(m_clientSocket, recvBuffer, recvBufferLength, 0);
    if (iResult > 0)
    {
        char* start = recvBuffer;

        std::vector<SPacketPayload> payloads;

        // As long as start continues to be a size we have more to read. Keep building payloads until we are done
        do
        {
            uint32_t offset = 0;
            if (!m_encryptionEnabled)
            {
                SPacketPayload payload = ReadUnecryptedPacket(start, offset);

                // Shift the start to the beginning of what would be the next packet
                start = start + (payload.m_size + offset);

                uint32_t packetId = payload.m_packetId;
                const bool result = pHandler->ProcessPacket(std::move(payload));
                if (!result)
                {
                    spdlog::warn("Error processing packet. Disconnecting connection. PacketId[{}] Address[{}]", packetId, GetRemoteAddress().c_str());
                
                    closesocket(m_clientSocket);
                    m_socketState = ESocketState::eSS_CLOSED;
                    m_clientSocket = INVALID_SOCKET;
                }
            }
            else
            {
                //TODO
            }
        } while (*start != 0);
        
        return true;
    }

    if (iResult == 0)
    {
        spdlog::debug("Client disconnected. Address[{}]", GetRemoteAddress());
        closesocket(m_clientSocket);
        m_socketState = ESocketState::eSS_CLOSED;
        return true;
    }

    const int error = WSAGetLastError();
    if(error == WSAEWOULDBLOCK)
    {
        // We're just waiting for something to receive. Break and we'll check if theres something next time
        return true;
    }

    // If we make it here something went wrong
    spdlog::error("Failed to receive packets from client. Error[{}] Address[{}]", error, GetRemoteAddress());
    
    closesocket(m_clientSocket);
    m_socketState = ESocketState::eSS_CLOSED;
    
    return false;
}

bool CClientConnection::SendPacket(SPacketPayload&& payload)
{
    int iResult = 0;
    if(!m_encryptionEnabled)
        iResult = send(m_clientSocket, payload.m_payload, payload.m_size, 0);
    else
    {
        // TODO
    }
    
    if (iResult == SOCKET_ERROR)
    {
        spdlog::error("Failed to send payload to client. Error[{}] Address[{}]", WSAGetLastError(), GetRemoteAddress());

        m_socketState = ESocketState::eSS_CLOSED;
        closesocket(m_clientSocket);
        return false;
    }
    
    return false;
}

SPacketPayload CClientConnection::ReadUnecryptedPacket(char* start, uint32_t& offset)
{
    // We take one away from the payload size since we read the packet id out immediately
    const uint32_t payloadSize = IPacket::DeserializeVarInt(start, offset);
    const uint32_t sizeOffset = offset;
            
    const uint32_t packetId = IPacket::DeserializeVarInt(start + offset, offset);
    const uint32_t packetIdSize = offset - sizeOffset;

    const uint32_t finalPayloadSize = payloadSize - packetIdSize;
            
    // Create a payload that will be routed to the relevant state handler
    SPacketPayload payload;
    payload.m_packetId = packetId;
    payload.m_size = finalPayloadSize;
    payload.m_startOffset = offset;

    payload.m_payload = new char[payloadSize + sizeOffset];
    memmove(payload.m_payload, start, payloadSize + sizeOffset);

    return payload;
}

char* CClientConnection::DecryptPacket(char* start)
{
    return nullptr;
}

char* CClientConnection::EncryptPacket(char* start)
{
    return nullptr;
}
