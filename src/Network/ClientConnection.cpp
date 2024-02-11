#include "pch.h"
#include "ClientConnection.h"

#include "IPacketHandler.h"
#include "PacketPayload.h"
#include "Packets/IPacket.h"
#include "spdlog/fmt/bin_to_hex.h"

#include <vector>

#include "Packets/PacketReader.h"
#include "Packets/PacketSizeCalc.h"

bool CClientConnection::RecvPackets(IPacketHandler* pHandler)
{
    MCPP_PROFILE_SCOPE()
    
    return m_socket.Recv([this, pHandler](char* bufferStart, uint32_t totalSize)
    {
        char* m_decryptedBuffer = nullptr;
        char* start;
        uint32_t totalOffset = 0;
        if (m_encryptionEnabled)
        {
            m_decryptedBuffer = reinterpret_cast<char*>(m_secret->DecryptPacket(reinterpret_cast<unsigned char*>(bufferStart), totalSize));
            start = m_decryptedBuffer;
        }
        else
            start = bufferStart;

        std::vector<SPacketPayload> payloads;

        // Keep processing packets until we've read the entire buffer
        while (totalOffset < totalSize) {
            SPacketPayload payload = ReadUnencryptedPacket(start, totalSize);

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

                m_socket.Stop();
                break;
            }
        }
        
        delete[] m_decryptedBuffer;
    });
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
        bool success;
        if(!m_encryptionEnabled)
            success = m_socket.Send(payload.m_payload, payload.m_size);
        else
        {
            int cipherLength = 0;
            char* encryptedPacket = reinterpret_cast<char*>(m_secret->EncryptPacket(reinterpret_cast<unsigned char*>(payload.m_payload), payload.m_size, cipherLength));
            success = m_socket.Send(encryptedPacket, cipherLength);
            delete[] encryptedPacket;
        }

        if(!success)
            return false;
    }
    
    return true;
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
        return {};
    
    payload.m_payload = new char[finalPayloadSize];
    memmove(payload.m_payload, start, finalPayloadSize);

    return payload;
}
