#pragma once
#include <string>

struct IRSAKeyPair
{
    virtual ~IRSAKeyPair() = default;
    
    virtual bool Initialise() = 0;
    virtual std::string Encrypt(std::string& input) const = 0;
    virtual std::string Decrypt(std::string& input) const = 0;

    virtual std::string GetAsnDerKey() const = 0;
};
