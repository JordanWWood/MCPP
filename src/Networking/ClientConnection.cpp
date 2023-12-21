﻿#include "ClientConnection.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include "Common/IPacketHandler.h"
#include "Common/PacketPayload.h"

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

    ZeroMemory(&recvBuffer, recvBufferLength);
    
    const int iResult = recv(m_clientSocket, recvBuffer, recvBufferLength, 0);
    if (iResult > 0)
    {
        char* start = recvBuffer;

        std::vector<SPacketPayload> payloads;

        // As long as start continues to be a size we have more to read. Keep building payloads until we are done
        do
        {
            // We take one away from the payload size since we read the packet id out immediately
            uint8_t payloadSize = (static_cast<uint8_t>(start[0]) - 1);
            uint8_t packetId = static_cast<uint8_t>(start[1]);

            // Create a payload that will be routed to the relevant state handler
            SPacketPayload payload;
            payload.m_packetId = packetId;
            payload.m_size = payloadSize;

            // If payload size is zero then this is a packet that has no payload. E.G. Status Request
            // No point allocating a payload if that is the case
            if (payloadSize > 0)
            {
                payload.m_payload = new char[payloadSize];

                // We trim the first two bytes (size & packetId) since we've already read that infomration
                memcpy(payload.m_payload, start + 2, payloadSize);
            }

            // Shift the start to the beginning of the next packet
            start = &start[payloadSize + 2];

            const bool result = pHandler->ProcessPacket(std::move(payload));
            if (!result)
            {
                closesocket(m_clientSocket);
                m_socketState = ESocketState::eSS_CLOSED;
                m_clientSocket = INVALID_SOCKET;
            }
        } while (*start != 0);
        
        return true;
    }

    if (iResult == 0)
    {
        spdlog::info("Client connection disconnected");
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
    spdlog::error("Recv failed with error {}", error);
    
    closesocket(m_clientSocket);
    m_socketState = ESocketState::eSS_CLOSED;
    
    return false;
}

bool CClientConnection::SendPacket(SPacketPayload&& payload)
{
    char* sendBuffer = new char[payload.m_size];
    memcpy(sendBuffer, payload.m_payload, payload.m_size);

    int iResult = send(m_clientSocket, sendBuffer, payload.m_size, 0);
    if (iResult == SOCKET_ERROR)
    {
        spdlog::error("Failed to send payload to client. Error[{}]", WSAGetLastError());

        m_socketState = ESocketState::eSS_CLOSED;
        closesocket(m_clientSocket);
        return false;
    }
    
    return false;
}
