#pragma once

#include <openssl/sha.h>

//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Official repository: https://github.com/AlexAndDad/gateway
//

// TODO this entire class needs to be updated to OpenSSL 3

struct SAuthHash
{
    SAuthHash()
    : ctx_ {}
    {
        SHA1_Init(&ctx_);
    }

    SAuthHash(SAuthHash const &) = delete;
    SAuthHash(SAuthHash &&)      = delete;
    SAuthHash &operator=(SAuthHash const &) = delete;
    SAuthHash &operator=(SAuthHash &&) = delete;
    ~SAuthHash() {}

    void Update(std::string in) { SHA1_Update(&ctx_, in.data(), in.size()); }

    std::string Finalise();

private:
    SHA_CTX ctx_;
};