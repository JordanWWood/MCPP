#pragma once

#include <openssl/evp.h>


//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Official repository: https://github.com/AlexAndDad/gateway
//

// TODO this entire class needs to be updated to OpenSSL 3

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