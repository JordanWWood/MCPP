﻿#include "pch.h"
#include "RSAKeyPair.h"

#include <string>
#include <openssl/x509.h>

#include "spdlog/spdlog.h"

CRSAKeyPair::~CRSAKeyPair()
{
    EVP_PKEY_free(m_pKey);
}

bool CRSAKeyPair::Initialise()
{
    int				ret = 0;
    BIGNUM			*bne = nullptr;

    int				bits = 1024;
    unsigned long	e = RSA_F4;
    
    bne = BN_new();
    ret = BN_set_word(bne,e);
    if(ret != 1)
    {
        BN_free(bne);
        
        spdlog::error("word error");
        return false;
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (ctx == nullptr) {
        BN_free(bne);
        spdlog::error("EVP_PKEY_CTX_new_id() failed");
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0 ||
        EVP_PKEY_CTX_set1_rsa_keygen_pubexp(ctx, bne) <= 0) {
        BN_free(bne);
        EVP_PKEY_CTX_free(ctx);
        spdlog::error("EVP_PKEY_keygen_init() or related functions failed");
        return false;
    } 

    if (EVP_PKEY_keygen(ctx, &m_pKey) <= 0) {
        BN_free(bne);
        EVP_PKEY_CTX_free(ctx);
        spdlog::error("EVP_PKEY_keygen() failed");
        return false;
    }
    
    unsigned char* derBuffer = new unsigned char[512];
    unsigned char* begin = derBuffer;

    // TODO this is deprecated figure out the snazzy new way to do this
    RSA* pRSA = EVP_PKEY_get1_RSA(m_pKey);
    int length = i2d_RSA_PUBKEY(pRSA, &derBuffer);

    m_asnDerPublicKey = std::string(begin, begin + length);

    // Clean up
    BN_free(bne);
    EVP_PKEY_CTX_free(ctx);
    RSA_free(pRSA);
    
    delete[] begin;

    return true;
}

std::string CRSAKeyPair::Encrypt(std::string& input) const
{
    // Convert string to unsigned char buffer
    const unsigned char* inputData = reinterpret_cast<const unsigned char*>(input.c_str());
    size_t inputLength = input.length();
    
    // Create the RSA public key context
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(m_pKey, nullptr);
    if (!ctx)
    {
        spdlog::error("EVP_PKEY_CTX_new() failed");
        return "";
    }

    // Initialize the encryption operation
    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        spdlog::error("EVP_PKEY_encrypt_init() failed");
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    // Calculate the size of the encrypted buffer
    size_t encryptedSize = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &encryptedSize, (const unsigned char*) inputData, inputLength) <= 0)
    {
        spdlog::error("EVP_PKEY_encrypt() failed");
        EVP_PKEY_CTX_free(ctx);
        return "";
    }
    
    // Perform the encryption
    unsigned char* cypherText = static_cast<unsigned char*>(OPENSSL_malloc(encryptedSize));
    int result = EVP_PKEY_encrypt(ctx, cypherText, &encryptedSize, (const unsigned char*) inputData, inputLength);
    if (result <= 0)
    {
        spdlog::error("EVP_PKEY_encrypt() failed. Result[{}]", result);
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    std::string finalString = std::string(reinterpret_cast<char*>(cypherText), encryptedSize);

    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(cypherText);

    // Convert the encrypted buffer to a string
    return finalString;
}

std::string CRSAKeyPair::Decrypt(std::string& input) const
{
    // Convert string to unsigned char buffer
    const unsigned char* inputData = reinterpret_cast<const unsigned char*>(input.c_str());
    size_t inputLength = input.length();

    // Calculate the size of the encrypted buffer
    size_t encryptedSize = EVP_PKEY_size(m_pKey);
    std::vector<unsigned char> decryptedBuffer(encryptedSize);

    // Create the RSA public key context
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(m_pKey, nullptr);
    if (!ctx)
    {
        spdlog::error("EVP_PKEY_CTX_new() failed");
        return "";
    }

    // Initialize the encryption operation
    if (EVP_PKEY_decrypt_init(ctx) <= 0)
    {
        spdlog::error("EVP_PKEY_encrypt_init() failed");
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    // Perform the encryption
    if (EVP_PKEY_decrypt(ctx, decryptedBuffer.data(), &encryptedSize, inputData, inputLength) <= 0)
    {
        spdlog::error("EVP_PKEY_encrypt() failed");
        EVP_PKEY_CTX_free(ctx);
        return "";
    }
    
    EVP_PKEY_CTX_free(ctx);

    return std::string(decryptedBuffer.data(), decryptedBuffer.data() + encryptedSize);
}
