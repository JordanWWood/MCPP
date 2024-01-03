#pragma once

#include <memory>
#include <string>

#include "IPacketHandler.h"
#include "HTTP/HTTPGet.h"

struct IRSAKeyPair;
enum class EClientState : uint8_t;
struct SPacketPayload;

struct IConnection;
using IConnectionPtr = std::shared_ptr<IConnection>;

class CMCPlayer : public IPacketHandler
{
public:
    CMCPlayer(IConnectionPtr pConnection, const std::shared_ptr<IRSAKeyPair>& pServerKey)
        : m_pConnection(std::move(pConnection))
        , m_pServerKey(pServerKey)
    {}
    
    void NetworkTick();
    virtual bool ProcessPacket(SPacketPayload&& payload) final;

    bool IsDead() const;
    std::string GetUsername() const { return m_username; }
    EClientState GetCurrentState() const { return m_state; }
private:
    bool HandleHandshake(SPacketPayload&& payload);
    bool HandleLogin(SPacketPayload&& payload);
    bool HandleStatus(SPacketPayload&& payload);

    bool SendEncryptionRequest();
    
    std::string m_username;
    
    IConnectionPtr m_pConnection{ nullptr };
    EClientState m_state { static_cast<EClientState>(0) };
    std::shared_ptr<IRSAKeyPair> m_pServerKey { nullptr };

    std::vector<CHTTPGet> m_runningGetRequest;

    std::string m_verifyToken;
};
