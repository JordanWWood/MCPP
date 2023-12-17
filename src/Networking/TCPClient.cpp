#include "TCPClient.h"

#include <cstdio>
#include <memory>
#include <string>

#include "Packets/BasePacket.h"

struct SHandShake : BasePacket // TODO packets
{
    SHandShake(char* start)
    {
        uint32_t offset = 0;
        m_protocolVersion = DeserializeVarInt(start, offset);
        m_address = DeserializeString(&start[offset], 32, offset);
        
    }
    
    uint32_t m_protocolVersion { 0 };
    std::string m_address;
    uint16_t m_port { 0 };
    uint8_t m_nextState { 0 };
};

CTCPClient::~CTCPClient()
{
    if(m_clientSocket != INVALID_SOCKET)
        closesocket(m_clientSocket);
}

TTCPClientBundlePtr CTCPClient::RecvPackets()
{
    TTCPClientBundlePtr bundle = std::make_shared<STCPClientBundle>();
    
    const int iResult = recv(m_clientSocket, bundle->m_recvBuffer, bundle->m_recvBufferLength, 0);
    if (iResult > 0)
    {
        // TODO log that we received something
    
        uint32_t payloadSize = static_cast<uint32_t>(bundle->m_recvBuffer[0]);
        uint32_t packetId = static_cast<uint32_t>(bundle->m_recvBuffer[1]);
        
        SHandShake hand_shake(&bundle->m_recvBuffer[2]);
        
        return bundle;
    }

    if (iResult == 0)
    {
        // We're done processing the queue. break
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
    WSACleanup();
    
    return nullptr;
}
