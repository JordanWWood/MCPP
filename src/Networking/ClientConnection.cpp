#include "pch.h"
#include "ClientConnection.h"

#include <vector>

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/types.h>

#include "Common/IPacketHandler.h"
#include "Common/PacketPayload.h"
#include "Common/Packets/IPacket.h"
#include "Encryption/AuthHash.h"

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
        char* start = recvBuffer;

        std::vector<SPacketPayload> payloads;

        // As long as start continues to be a size we have more to read. Keep building payloads until we are done
        do
        {
            uint32_t offset = 0;
            if (!m_encryptionEnabled)
            {
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
        MCLog::error("Failed to send payload to client. Error[{}] Address[{}]", WSAGetLastError(), GetRemoteAddress());

        m_socketState = ESocketState::eSS_CLOSED;
        closesocket(m_clientSocket);
        return false;
    }
    
    return false;
}

std::string CClientConnection::GenerateHexDigest(std::string publicKey, std::string sharedSecret)
{
    SAuthHash hasher;
    hasher.Update("");
    hasher.Update(sharedSecret);
    hasher.Update(publicKey);
    
    return hasher.Finalise();
}

SPacketPayload CClientConnection::ReadUnencryptedPacket(char* start, uint32_t& offset)
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

unsigned char* CClientConnection::DecryptPacket(unsigned char* start, int length) const
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cfb8(), nullptr, reinterpret_cast<const unsigned char*>(m_aesKey.c_str()),
                       reinterpret_cast<const unsigned char*>(m_aesKey.c_str));
    
    unsigned char* decryptedText = new unsigned char[length + EVP_MAX_BLOCK_LENGTH];
    int decryptedLength = 0;
    if (EVP_DecryptUpdate(ctx, start, &length, decryptedText, decryptedLength) != 1) {
        MCLog::error("Error decrypting data");
        EVP_CIPHER_CTX_free(ctx);
        return nullptr;
    }

    EVP_DecryptFinal_ex(ctx, start + length, &length);

    EVP_CIPHER_CTX_free(ctx);
    
    return nullptr;
}

unsigned char* CClientConnection::EncryptPacket(unsigned char* start, int length, int& outCipherLength) const
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cfb8(), nullptr, reinterpret_cast<const unsigned char*>(m_aesKey.c_str()),
                       reinterpret_cast<const unsigned char*>(m_aesKey.c_str()));

    unsigned char* cipherText = new unsigned char[length + EVP_MAX_BLOCK_LENGTH];
    int cipherLength = 0;
    if(EVP_EncryptUpdate(ctx, start, &length, cipherText, cipherLength) != 0)
    {
        MCLog::error("Error encrypting data");
        EVP_CIPHER_CTX_free(ctx);
        return nullptr;
    }

    EVP_EncryptFinal_ex(ctx, start + length, &length);

    EVP_CIPHER_CTX_free(ctx);
    return cipherText;
}
