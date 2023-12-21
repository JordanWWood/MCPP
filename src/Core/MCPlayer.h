#pragma once

#include <memory>
#include <string>

#include "Common\IPacketHandler.h"
#include "Common\IConnection.h"

enum class EClientState : uint8_t;
struct SPacketPayload;
struct IConnection;

using IConnectionPtr = std::shared_ptr<IConnection>;

class CMCPlayer : public IPacketHandler
{
public:
    CMCPlayer(const IConnectionPtr& pConnection)
        : m_pConnection(pConnection)
    {}
    
    void RecvPackets();
    virtual bool ProcessPacket(SPacketPayload&& payload) final;

    bool IsDead() const { return m_pConnection->IsSocketClosed(); }
    std::string GetUsername() const { return m_username; }
private:
    bool HandleHandshake(SPacketPayload&& payload);
    bool HandleLogin(SPacketPayload&& payload);
    bool HandleStatus(SPacketPayload&& payload);

    std::string m_username;
    
    IConnectionPtr m_pConnection{ nullptr };
    EClientState m_state { static_cast<EClientState>(0) };
};
