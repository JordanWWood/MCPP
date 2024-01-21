#pragma once

#include <memory>
#include <string>

#include "IPacketHandler.h"
#include "Uuid.h"

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

    /////////////////////////////////////////////////////////////////////
    // IPacketHandler
    virtual void NetworkTick() final;
    virtual bool ProcessPacket(SPacketPayload&& payload) final;
    // ~IPacketHandler
    /////////////////////////////////////////////////////////////////////
    
    bool IsDead() const override;
    
    const std::string& GetUsername() const { return m_username; }
    const CUUID& GetUUID() const { return m_uuid; }
    const std::string& GetRemoteAddress() const;
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
    CUUID m_uuid;
    
    std::string m_verifyToken;
};
