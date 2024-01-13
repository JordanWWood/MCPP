#include "pch.h"
#include "ClientConnection.h"

#include "IPacketHandler.h"
#include "PacketPayload.h"
#include "Packets/IPacket.h"
#include "spdlog/fmt/bin_to_hex.h"

#include <vector>
#include <openssl/err.h>

#ifdef _WIN32
#include <WinSock2.h>
#endif

#define DEFAULT_BUFLEN 512

CClientConnection::~CClientConnection()
{
    if(m_clientSocket != INVALID_SOCKET)
        closesocket(m_clientSocket);
}

bool CClientConnection::RecvPackets(IPacketHandler* pHandler)
{
    OPTICK_EVENT();

    char recvBuffer[DEFAULT_BUFLEN];
    constexpr int recvBufferLength{ DEFAULT_BUFLEN };
    memset(&recvBuffer, 0, recvBufferLength);
    
    const int iResult = recv(m_clientSocket, recvBuffer, recvBufferLength, 0);
    if (iResult > 0)
    {
        char* m_decryptedBuffer = nullptr;
        char* start;
        if (m_encryptionEnabled)
        {
            m_decryptedBuffer = reinterpret_cast<char*>(m_secret->DecryptPacket(reinterpret_cast<unsigned char*>(recvBuffer), recvBufferLength));
            start = m_decryptedBuffer;
        }
        else
            start = recvBuffer;

        std::vector<SPacketPayload> payloads;

        // As long as start continues to be a size we have more to read. Keep building payloads until we are done
        do
        {
            uint32_t offset = 0;
            SPacketPayload payload = ReadUnencryptedPacket(start, offset);
            
            // Shift the start to the beginning of what would be the next packet
            start = start + (payload.m_size + offset);
            uint32_t packetId = payload.m_packetId;
            
            const bool result = pHandler->ProcessPacket(std::move(payload));
            if (!result)
            {
                MCLog::warn("Error processing packet. Disconnecting connection. PacketId[{}] Address[{}]", packetId, GetRemoteAddress().c_str());
               
                closesocket(m_clientSocket);
                m_socketState = ESocketState::eSS_CLOSED;
                m_clientSocket = INVALID_SOCKET;
                break;
            }
        } while (*start != 0);
        
        return true;
    }

    if (iResult == 0)
    {
        MCLog::debug("Client disconnected. Address[{}]", GetRemoteAddress());
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
    MCLog::error("Failed to receive packets from client. Error[{}] Address[{}]", error, GetRemoteAddress());
    
    closesocket(m_clientSocket);
    m_socketState = ESocketState::eSS_CLOSED;
    
    return false;
}

void CClientConnection::QueuePacket(SPacketPayload&& payload)
{
    OPTICK_EVENT();
    m_queuedSends.enqueue(std::move(payload));
}

bool CClientConnection::SendQueuedPackets()
{
    OPTICK_EVENT();
    
    SPacketPayload payload;
    while(m_queuedSends.try_dequeue(payload))
    {
        int iResult;
        if(!m_encryptionEnabled)
            iResult = send(m_clientSocket, payload.m_payload, payload.m_size, 0);
        else
        {
            int cipherLength = 0;
            char* encryptedPacket = reinterpret_cast<char*>(m_secret->EncryptPacket(reinterpret_cast<unsigned char*>(payload.m_payload), payload.m_size, cipherLength));
            iResult = send(m_clientSocket, encryptedPacket, cipherLength, 0);
            delete[] encryptedPacket;
        }
    
        if (iResult == SOCKET_ERROR)
        {
            MCLog::error("Failed to send payload to client. Error[{}] Address[{}]", WSAGetLastError(), GetRemoteAddress());

            m_socketState = ESocketState::eSS_CLOSED;
            closesocket(m_clientSocket);
            return false;
        }
    }
    
    return false;
}

SPacketPayload CClientConnection::ReadUnencryptedPacket(char* start, uint32_t& offset)
{
    OPTICK_EVENT()
    
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
