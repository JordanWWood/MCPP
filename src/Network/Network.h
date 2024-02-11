#pragma once

#include <map>

#include "IConfiguration.h"
#include "IGlobalEnvironment.h"
#include "INetwork.h"
#include "IPacketHandler.h"
#include "TCPSocket.h"

class CNetwork final : public INetwork
{
public:
    CNetwork(uint16_t hostPort);
    ~CNetwork() override;

    /////////////////////////////////////////////////////////////////////
    // INetwork
    void RegisterPacketHandler(std::weak_ptr<IPacketHandler> handlerWeakPtr) override;
    void RegisterConnectionCallback(void* pCreator, std::function<void(const IConnectionPtr& pConnection)>&& functor) override;
    void UnregisterConnectionCallback(void* creator) override;

    bool HasShutdown() const override { return m_shutdown; }

    std::shared_ptr<IRSAKeyPair> GetServerKeyPair() override { return m_pKeyPair; }
    std::string GenerateHexDigest(std::string publicKey, std::string sharedSecret) override;
    // ~INetwork
    /////////////////////////////////////////////////////////////////////
    
    void NetworkTick();

private:
    CTCPSocket m_listenSocket;

    std::vector<IConnectionPtr> m_activeConnections;
    std::vector<std::weak_ptr<IPacketHandler>> m_packetHandlers;
    std::thread m_networkThread;
    std::mutex m_networkLock;
    
    std::map<void*, std::function<void(IConnectionPtr pConnection)>> m_connectionCallbacks;
    std::atomic_bool m_shutdown{ false };

    std::shared_ptr<IRSAKeyPair> m_pKeyPair;
};
