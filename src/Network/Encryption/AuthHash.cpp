#include "pch.h"
#include "AuthHash.h"

#include <openssl/bn.h>
#include <openssl/sha.h>

SAuthHash::SAuthHash(): m_pCtx {}
{
    m_pCtx = EVP_MD_CTX_new();
    m_pMd = EVP_sha1();

    EVP_DigestInit_ex(m_pCtx, m_pMd, nullptr);
}

SAuthHash::~SAuthHash()
{
    EVP_MD_CTX_free(m_pCtx);
}

void SAuthHash::Update(std::string in) const
{
    OPTICK_EVENT();
    EVP_DigestUpdate(m_pCtx, in.data(), in.size());
}

std::string SAuthHash::Finalise() const
{
    OPTICK_EVENT();

    auto result = std::string();

    unsigned char hash[SHA_DIGEST_LENGTH];
    unsigned int hashLength;
    
    EVP_DigestFinal_ex(m_pCtx, hash, &hashLength);

    // convert has to bignum
    BIGNUM *bn = BN_bin2bn(hash, hashLength, nullptr);

    // reset the hasher for next use
    EVP_DigestInit_ex(m_pCtx, m_pMd, nullptr);

    // check for "negative" value
    if (BN_is_bit_set(bn, 159))
    {
        result += '-';

        // perform 1's compliment on the bignum's bits
        auto tmp = std::vector< unsigned char >(BN_num_bytes(bn));
        BN_bn2bin(bn, tmp.data());
        std::ranges::transform(tmp, tmp.begin(), [](unsigned char b) { return ~b; });
        BN_bin2bn(tmp.data(), tmp.size(), bn);

        // add 1 "as-if" 2's compliment

        BN_add_word(bn, 1);
    }

    // convert to hex
    auto hex = BN_bn2hex(bn);

    // remove any leading zeroes except the last
    auto view = std::string_view(hex);
    while (!view.empty() && view[0] == '0')
        view = view.substr(1);

    // append the hex to the result
    result.append(view.begin(), view.end());
    OPENSSL_free(hex);
    BN_free(bn);

    // convert the hex to lower case
    std::ranges::transform(result, result.begin(),
                           [](unsigned char c){ return std::tolower(c); });
    
    return result;
}
