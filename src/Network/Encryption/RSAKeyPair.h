#pragma once
#include <string>

#include "Encryption/IRSAKeyPair.h"

struct evp_pkey_st;
typedef evp_pkey_st EVP_PKEY;

class CRSAKeyPair : public IRSAKeyPair
{
public:
    CRSAKeyPair() = default;
    ~CRSAKeyPair() override;
    
    virtual bool Initialise() override;
    virtual std::string Encrypt(std::string& input) const override;
    virtual std::string Decrypt(std::string& input) const override;
    
    virtual std::string GetAsnDerKey() const override { return m_asnDerPublicKey; }

private:
    EVP_PKEY* m_pKey { nullptr };
    std::string m_asnDerPublicKey;
};
