#pragma once

class CSharedSecret
{
public:
    CSharedSecret(std::string&& secret)
        : m_secret(std::move(secret))
    {}
    
    unsigned char* DecryptPacket(unsigned char* start, int length) const;
    unsigned char* EncryptPacket(unsigned char* start, int length, int& outCipherLength) const;

private:
    std::string m_secret;
};
