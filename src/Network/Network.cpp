#include "pch.h"
#include "Network.h"

#include "ClientConnection.h"
#include "IConnection.h"
#include "Encryption/AuthHash.h"
#include "Encryption/RSAKeyPair.h"
#include "TCPSocket.h"

#define NETWORK_THREAD_UPDATE_RATE 120
using TNetworkThreadFrame = std::chrono::duration<int64_t, std::ratio<1, NETWORK_THREAD_UPDATE_RATE>>;

CONFIG_GROUP_BEGIN(CNetworkConfig, network, base)
CONFIG_GROUP_MEMBER(std::string, host, "locahost")
CONFIG_GROUP_MEMBER(int, port, 25565)
CONFIG_GROUP_END()

static void NetworkThread(CNetwork* instance)
{
    OPTICK_THREAD("Network Thread");

    instance->NetworkTick();
}

CNetwork::CNetwork(uint16_t hostPort)
    : m_listenSocket(std::make_unique<CTCPSocket>(eSF_Passive | eSF_Bind | eSF_Listen, "", hostPort))
{
    REGISTER_CONFIG_GROUP(CNetworkConfig)

    m_config = static_cast<CNetworkConfig*>(IGlobalEnvironment::Get().GetConfigManager().lock()->GetConfigGroup<config::network::base>());

#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(iResult != 0)
    {
        // TODO Logging
        return;
    }
#endif

    m_networkThread = std::thread(NetworkThread, this);

    MCLog::info("Generating RSA key pair");
    m_pKeyPair = std::make_shared<CRSAKeyPair>();
    m_pKeyPair->Initialise();
}

CNetwork::~CNetwork()
{
    m_shutdown = true;

    // Wait for the network thread to exit
    m_networkThread.join();

#ifdef _WIN32
    WSACleanup();
#endif
}

void CNetwork::RegisterPacketHandler(std::weak_ptr<IPacketHandler> handlerWeakPtr)
{
    // TODO This could probably do with some additional safety
    m_packetHandlers.push_back(handlerWeakPtr);
}

void CNetwork::RegisterConnectionCallback(void* creator, std::function<void(const IConnectionPtr& pConnection)>&& functor)
{
    m_connectionCallbacks.emplace(creator, std::move(functor));
}

void CNetwork::UnregisterConnectionCallback(void* creator)
{
    m_connectionCallbacks.erase(creator);
}

std::string CNetwork::GenerateHexDigest(std::string publicKey, std::string sharedSecret)
{
    MCPP_PROFILE_SCOPE()
    
    SAuthHash hasher;
    hasher.Update(sharedSecret);
    hasher.Update(publicKey);
    
    return hasher.Finalise();
}

void CNetwork::NetworkTick()
{
    m_shutdown = !m_listenSocket->Start();

    auto nextFrame = std::chrono::high_resolution_clock::now() + TNetworkThreadFrame{1};
    while (!m_shutdown)
    {
        {
            MCPP_PROFILE_NAMED_SCOPE("Network Update")

            // TODO we should reestablish the listen socket if it closes. For now the application just exits
            if (m_listenSocket->IsClosed())
                break;

            // Grab the network lock for any work here
            std::lock_guard lock(m_networkLock);

            // Network update
            // 1 Accept new connections
            if (IConnectionPtr pConnection = m_listenSocket->Accept<CClientConnection>())
            {
                for (auto& callback : m_connectionCallbacks)
                    callback.second(pConnection);

                m_activeConnections.push_back(pConnection);
            }
         
            // 2 Update current connections
            // TODO I'm not a fan of this packet handler pattern. An alternative would be ideal. Perhaps a map of handlers for connections?
            for (std::vector<std::weak_ptr<IPacketHandler>>::iterator it = m_packetHandlers.begin(); it != m_packetHandlers.end();)
            {
                std::shared_ptr<IPacketHandler> pHandler = it->lock();
                if(!pHandler || pHandler->IsDead())
                {
                    // TODO we need some better logging here. Like what we had when we were in CMCPlayer
                    MCLog::info("Disconnecting client");
                    
                    // Handler has been deleted
                    it = m_packetHandlers.erase(it);
                    continue;
                }
                
                pHandler->NetworkTick();
                ++it;
            }

            // 3 Send packets if the main thread frame has ended
            //   Also remove any stale connections
            for (std::vector<IConnectionPtr>::iterator it = m_activeConnections.begin(); it != m_activeConnections.end();)
            {
                const IConnectionPtr& pConnection = *it;
                if(pConnection->IsSocketClosed())
                {
                    it = m_activeConnections.erase(it);
                    continue;
                }

                switch (pConnection->GetConnectionType())
                {
                case EConnectionType::eCT_Client: {
                        CClientConnection* pClientConnection = dynamic_cast<CClientConnection*>(pConnection.get());
                        pClientConnection->SendQueuedPackets();
                    } break;
                case EConnectionType::eCT_Server:
                    break;
                }

                ++it;
            }
        }
        
        {
            MCPP_PROFILE_NAMED_SCOPE("Sleep")
            std::this_thread::sleep_until(nextFrame);
            nextFrame += TNetworkThreadFrame{1};
        }
    }
}
