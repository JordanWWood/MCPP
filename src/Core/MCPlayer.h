#pragma once

#include <memory>
#include <string>

#include "IPacketHandler.h"

struct IRSAKeyPair;
enum class EClientState : uint8_t;
struct SPacketPayload;

struct IConnection;
using IConnectionPtr = std::shared_ptr<IConnection>;

class CMCPlayer : public IPacketHandler
{
public:
    CMCPlayer(IConnectionPtr pConnection)
        : m_pConnection(std::move(pConnection))
    {}
    
    virtual void NetworkTick() final;
    virtual bool ProcessPacket(SPacketPayload&& payload) final;

    bool IsDead() const override;
    
    std::string GetUsername() const { return m_username; }
    EClientState GetCurrentState() const { return m_state; }
private:
    bool HandleHandshake(SPacketPayload&& payload);
    bool HandleLogin(SPacketPayload&& payload);
    bool HandleStatus(SPacketPayload&& payload);

    bool SendEncryptionRequest();

private:
    std::string m_username;
    
    IConnectionPtr m_pConnection{ nullptr };
    EClientState m_state { static_cast<EClientState>(0) };
    
    std::string m_verifyToken;
};
