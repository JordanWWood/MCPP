#include "MCServer.h"

#include "MCPlayer.h"

#include "Common/Packets/IPacket.h"

#include <chrono>
#include <spdlog/spdlog.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>

#define MAIN_THREAD_UPDATE_RATE 20
#define NETWORK_THREAD_UPDATE_RATE 120

using TMainThreadFrame = std::chrono::duration<int64_t, std::ratio<1, MAIN_THREAD_UPDATE_RATE>>;
using TNetworkThreadFrame = std::chrono::duration<int64_t, std::ratio<1, NETWORK_THREAD_UPDATE_RATE>>;

#define THREAD_UPDATE_BEGIN(frame) auto nextFrame = std::chrono::system_clock::now() + frame{1}
#define THREAD_UPDATE_END() std::this_thread::sleep_until(nextFrame)

static void NetworkThread(CMCServer* mcServer)
{
    mcServer->NetworkRun();
}

CMCServer::CMCServer(uint16_t port)
    : m_pTcpServer(std::make_unique<CTCPServer>(port))
{
}

std::string decryptString(const std::string& input, EVP_PKEY* privateKey)
{
    // Convert string to unsigned char buffer
    const unsigned char* inputData = reinterpret_cast<const unsigned char*>(input.c_str());
    size_t inputLength = input.length();

    // Calculate the size of the encrypted buffer
    size_t encryptedSize = EVP_PKEY_size(privateKey);
    unsigned char* outBuffer = new unsigned char[encryptedSize];
    
    // Create the RSA public key context
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privateKey, nullptr);
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
    if (EVP_PKEY_decrypt(ctx, outBuffer, &encryptedSize, inputData, inputLength) <= 0)
    {
        spdlog::error("EVP_PKEY_encrypt() failed");
        EVP_PKEY_CTX_free(ctx);
        return "";
    }
    EVP_PKEY_CTX_free(ctx);


    std::string out = std::string(outBuffer, outBuffer + encryptedSize);
    delete[] outBuffer;
    
    // Convert the encrypted buffer to a string
    return out;
}

std::string encryptString(const std::string& input, EVP_PKEY* publicKey)
{
    // Convert string to unsigned char buffer
    const unsigned char* inputData = reinterpret_cast<const unsigned char*>(input.c_str());
    size_t inputLength = input.length();

    // Calculate the size of the encrypted buffer
    size_t encryptedSize = EVP_PKEY_size(publicKey);
    std::vector<unsigned char> encryptedBuffer(encryptedSize);

    // Create the RSA public key context
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(publicKey, nullptr);
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

    // Set the padding mode (RSA_PKCS1_PADDING is commonly used)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
    {
        spdlog::error("EVP_PKEY_CTX_set_rsa_padding() failed");
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    // Perform the encryption
    if (EVP_PKEY_encrypt(ctx, encryptedBuffer.data(), &encryptedSize, inputData, inputLength) <= 0)
    {
        spdlog::error("EVP_PKEY_encrypt() failed");
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    EVP_PKEY_CTX_free(ctx);

    // Convert the encrypted buffer to a string
    return std::string(reinterpret_cast<char*>(encryptedBuffer.data()), encryptedSize);
}

bool CMCServer::Init()
{
    spdlog::info("Initialising MC Server");
    m_networkThread = std::thread(NetworkThread, this);

    int				ret = 0;
    BIGNUM			*bne = nullptr;
    BIO				*bp_public = nullptr, *bp_private = nullptr;

    int				bits = 1024;
    unsigned long	e = RSA_F4;
    
    bne = BN_new();
    ret = BN_set_word(bne,e);
    if(ret != 1)
    {
        BIO_free_all(bp_public);
        BIO_free_all(bp_private);
        BN_free(bne);
        
        spdlog::error("word error");
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
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

    EVP_PKEY *pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        BN_free(bne);
        EVP_PKEY_CTX_free(ctx);
        spdlog::error("EVP_PKEY_keygen() failed");
        return false;
    }

    
    rsa_st* pRSA = EVP_PKEY_get1_RSA(pkey);
    
    EVP_PKEY* publicKey = pkey; // Assuming pkey is the public key obtained from key generation
    std::string inputString = "Your string to be encrypted";
    std::string encryptedString = encryptString(inputString, publicKey);

    if (!encryptedString.empty()) {
        // The encrypted string is now in encryptedString
        // You can do something with it, send it over the network, store it, etc.
    }

    std::string decryptedString = decryptString(encryptedString, pkey);

    unsigned char* derBuffer = new unsigned char[2048];
    PKCS8_PRIV_KEY_INFO* pkcsKeyInfo = EVP_PKEY2PKCS8(pkey);
    if (i2d_PKCS8_PRIV_KEY_INFO(pkcsKeyInfo, &derBuffer) <= 0)
    {
        spdlog::error("i2d_PKCS8_PRIV_KEY_INFO() faule")
    }
    
    // Clean up
    BN_free(bne);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    PKCS8_PRIV_KEY_INFO_free(pkcsKeyInfo);
    
    return true;
}

bool CMCServer::Run()
{
    THREAD_UPDATE_BEGIN(TMainThreadFrame);

    

    THREAD_UPDATE_END();

    return m_quit == false;
}

void CMCServer::NetworkRun()
{
    m_quit = !m_pTcpServer->Listen();

    while (!m_quit)
    {
        THREAD_UPDATE_BEGIN(TNetworkThreadFrame);

        // TODO we should reestablish the listen socket if it closes. For now the application just exits
        if (m_pTcpServer->IsSocketClosed())
            break;

        {
            std::lock_guard<std::mutex> lock(m_networkLock); 
            
            // Network update
            // 1 Accept new connections
            if (IConnectionPtr pClient = m_pTcpServer->AcceptConnection())
                m_players.emplace_back(pClient);

            // 2 Process packets
            // TODO move to own threads
            for (std::vector<CMCPlayer>::iterator it = m_players.begin(); it != m_players.end();)
            {
                CMCPlayer& player = *it;
                player.RecvPackets();

                if (player.IsDead())
                {
                    if (player.GetCurrentState() >= EClientState::eCS_Login)
                        spdlog::info("Client has disconnected. Username[{}] State[{}]", player.GetUsername(), static_cast<uint32_t>(player.GetCurrentState()));
                    else
                        spdlog::info("Server list ping disconnected. State[{}]", static_cast<uint32_t>(player.GetCurrentState()));
                    
                    it = m_players.erase(it); // This client is no longer connected. Remove it
                    continue;
                }

                ++it;
            }
        }

        THREAD_UPDATE_END();
    }

    m_quit = true;
}