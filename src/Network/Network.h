#pragma once

#include <map>

#include "INetwork.h"
#include "IPacketHandler.h"
#include "TCPServer.h"

class CNetwork final : public INetwork
{
public:
    CNetwork();

    /////////////////////////////////////////////////////////////////////
    // INetwork
    ITCPServer* GetTCPServer() override { return &m_tcpServer; }
    
    void RegisterPacketHandler(std::weak_ptr<IPacketHandler> handlerWeakPtr) override;
    void RegisterConnectionCallback(void* pCreator, std::function<void(const IConnectionPtr& pConnection)>&& functor) override;
    void UnregisterConnectionCallback(void* creator) override;

    std::shared_ptr<IRSAKeyPair> GetServerKeyPair() override { return m_pKeyPair; }
    std::string GenerateHexDigest(std::string publicKey, std::string sharedSecret) override;
    // ~INetwork
    /////////////////////////////////////////////////////////////////////
    
    void NetworkTick();

private:
    CTCPServer m_tcpServer;

    std::vector<std::weak_ptr<IPacketHandler>> m_packetHandlers;
    std::thread m_networkThread;
    std::mutex m_networkLock;
    
    std::map<void*, std::function<void(IConnectionPtr pConnection)>> m_connectionCallbacks;
    std::atomic_bool m_quit = false;

    std::shared_ptr<IRSAKeyPair> m_pKeyPair;
};
