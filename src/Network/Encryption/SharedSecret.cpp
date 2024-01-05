#include "pch.h"
#include "SharedSecret.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/types.h>

unsigned char* CSharedSecret::DecryptPacket(unsigned char* start, int length) const
{
    OPTICK_EVENT();
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_cfb8(), nullptr, reinterpret_cast<const unsigned char*>(m_secret.c_str()),
                       reinterpret_cast<const unsigned char*>(m_secret.c_str()));
    
    unsigned char* decryptedText = new unsigned char[length];
    int decryptedLength = 0;
    if (EVP_DecryptUpdate(ctx, decryptedText, &decryptedLength, start, length) <= 0) {
        MCLog::error("Error decrypting data");
        EVP_CIPHER_CTX_free(ctx);
        return nullptr;
    }

    EVP_DecryptFinal_ex(ctx, start + length, &length);

    EVP_CIPHER_CTX_free(ctx);
    
    return decryptedText;
}

unsigned char* CSharedSecret::EncryptPacket(unsigned char* start, int length, int& outCipherLength) const
{
    OPTICK_EVENT();
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cfb8(), nullptr, reinterpret_cast<const unsigned char*>(m_secret.c_str()),
                       reinterpret_cast<const unsigned char*>(m_secret.c_str()));

    unsigned char* cipherText = new unsigned char[length + EVP_MAX_BLOCK_LENGTH];
    int iResult = EVP_EncryptUpdate(ctx, cipherText, &outCipherLength, start, length);
    if(iResult <= 0)
    {
        uint64_t error = ERR_get_error();
        if (error != 0)
        {
            MCLog::error("Error encrypting data. Code[{}]", error);
            EVP_CIPHER_CTX_free(ctx);
            return nullptr;
        }
    }

    EVP_EncryptFinal_ex(ctx, start + length, &length);

    EVP_CIPHER_CTX_free(ctx);
    return cipherText;
}