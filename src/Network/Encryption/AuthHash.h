#pragma once

#include <openssl/evp.h>

struct SAuthHash
{
    SAuthHash();
    ~SAuthHash();
    
    SAuthHash(SAuthHash const &) = delete;
    SAuthHash(SAuthHash &&)      = delete;
    SAuthHash &operator=(SAuthHash const &) = delete;
    SAuthHash &operator=(SAuthHash &&) = delete;
    
    void Update(std::string in) const;

    std::string Finalise() const;

private:
    EVP_MD_CTX* m_pCtx;
    const EVP_MD* m_pMd;
};