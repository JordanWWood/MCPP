#include "pch.h"
#include "ClientConnection.h"

#include "IPacketHandler.h"
#include "PacketPayload.h"
#include "Packets/IPacket.h"
#include "spdlog/fmt/bin_to_hex.h"

#include <vector>
#include <openssl/err.h>

#include "Packets/PacketReader.h"
#include "Packets/PacketSizeCalc.h"

#define DEFAULT_BUFLEN 512

CClientConnection::~CClientConnection()
{
    if(m_clientSocket != INVALID_SOCKET)
        CLOSE_SOCKET(m_clientSocket);
}

bool CClientConnection::RecvPackets(IPacketHandler* pHandler)
{
    MCPP_PROFILE_SCOPE()

    char recvBuffer[DEFAULT_BUFLEN];
    constexpr int recvBufferLength{ DEFAULT_BUFLEN };
    memset(&recvBuffer, 0, recvBufferLength);
    
    const int iResult = recv(m_clientSocket, recvBuffer, recvBufferLength, 0);
    if (iResult > 0)
    {
        char* m_decryptedBuffer = nullptr;
        char* start;
        uint32_t totalOffset = 0;
        if (m_encryptionEnabled)
        {
            m_decryptedBuffer = reinterpret_cast<char*>(m_secret->DecryptPacket(reinterpret_cast<unsigned char*>(recvBuffer), iResult));
            start = m_decryptedBuffer;
        }
        else
            start = recvBuffer;

        std::vector<SPacketPayload> payloads;

        // Keep processing packets until we've read the entire buffer
        while (totalOffset < iResult) {
            SPacketPayload payload = ReadUnencryptedPacket(start, iResult);

            if(!payload.m_payload)
                break;
            
            // Shift the start to the beginning of what would be the next packet
            start += payload.m_size;
            uint32_t packetId = payload.m_packetId;

            // Keep track of how much we've read so far
            totalOffset += payload.m_size;
            
            const bool result = pHandler->ProcessPacket(std::move(payload));
            if (!result)
            {
                MCLog::warn("Error processing packet. Disconnecting connection. PacketId[{}] Address[{}]", packetId, GetRemoteAddress().c_str());
               
                CLOSE_SOCKET(m_clientSocket);
                m_socketState = ESocketState::eSS_CLOSED;
                m_clientSocket = INVALID_SOCKET;
                break;
            }
        }
        
        delete[] m_decryptedBuffer;
        return true;
    }

    if (iResult == 0)
    {
        MCLog::debug("Client disconnected. Address[{}]", GetRemoteAddress());
        CLOSE_SOCKET(m_clientSocket);
        m_socketState = ESocketState::eSS_CLOSED;
        return true;
    }

    const int error = GET_SOCKET_ERR();
    if(error == WOULD_BLOCK)
    {
        // We're just waiting for something to receive. Break and we'll check if theres something next time
        return true;
    }

    // If we make it here something went wrong
    MCLog::error("Failed to receive packets from client. Error[{}] Address[{}]", error, GetRemoteAddress());
    
    CLOSE_SOCKET(m_clientSocket);
    m_socketState = ESocketState::eSS_CLOSED;
    
    return false;
}

void CClientConnection::QueuePacket(SPacketPayload&& payload)
{
    MCPP_PROFILE_SCOPE()
    m_queuedSends.enqueue(std::move(payload));
}

bool CClientConnection::SendQueuedPackets()
{
    MCPP_PROFILE_SCOPE()
    
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
            MCLog::error("Failed to send payload to client. Error[{}] Address[{}]", GET_SOCKET_ERR(), GetRemoteAddress());

            m_socketState = ESocketState::eSS_CLOSED;
            CLOSE_SOCKET(m_clientSocket);
            return false;
        }
    }
    
    return false;
}

SPacketPayload CClientConnection::ReadUnencryptedPacket(char* start, uint32_t maxSize)
{
    MCPP_PROFILE_SCOPE()
    
    CPacketReader reader(start);
    int payloadSize = 0;
    reader.OnVarInt(payloadSize);

    CPacketSizeCalc sizeCalc;
    sizeCalc.OnVarInt(payloadSize);
    
    int packetId = 0;
    reader.OnVarInt(packetId);
    const uint32_t finalPayloadSize = payloadSize + sizeCalc.GetFullSize();
    
    // Create a payload that will be routed to the relevant state handler
    SPacketPayload payload;
    payload.m_packetId = packetId;
    payload.m_size = finalPayloadSize;

    if(finalPayloadSize > maxSize)
        return SPacketPayload();
    
    payload.m_payload = new char[finalPayloadSize];
    memmove(payload.m_payload, start, finalPayloadSize);

    return payload;
}
