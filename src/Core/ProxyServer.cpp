#include "pch.h"

#include "ProxyServer.h"
#include "MCPlayer.h"

#include "IGlobalEnvironment.h"
#include "IConfigurationManager.h"

struct SServer {
    std::string address{ "127.0.0.1" };
    int port{ 25565 };
};
CONFIG_STRUCT_BEGIN(SServer)
CONFIG_STRUCT_MEMBER(address);
CONFIG_STRUCT_MEMBER(port);
CONFIG_STRUCT_END()

CONFIG_GROUP_BEGIN_NO_NAMESPACE(CProxyServerConfig, proxy)
CONFIG_GROUP_MEMBER(bool, IsOnline, true)
CONFIG_GROUP_MEMBER(std::vector<SServer>, Servers, { CONCAT({}, {}, {}) })
CONFIG_GROUP_END()

CProxyServer::CProxyServer()
    : m_quit(false)
{
    MCPP_PROFILE_SCOPE()
    // Register the config group for the proxy server
    IGlobalEnvironment::Get().GetConfigManager().lock()->RegisterConfigGroup(new CProxyServerConfig());
    
    // Initialize the player list
    m_players.reserve(10); // Reserve space for 10 players initially
}

bool CProxyServer::Init()
{
    MCPP_PROFILE_SCOPE()

    IGlobalEnvironment::Get().GetNetwork().lock()->RegisterConnectionCallback(this, [this](const IConnectionPtr& pConnection) {
        std::lock_guard lock(m_playerLock);
        const std::shared_ptr<CMCPlayer> pPlayer = m_players.emplace_back(std::make_shared<CMCPlayer>(pConnection));
        IGlobalEnvironment::Get().GetNetwork().lock()->RegisterPacketHandler(pPlayer);
    });
    
    return true;
}

bool CProxyServer::Run()
{
    MCPP_PROFILE_SCOPE()

    {
        std::lock_guard lock(m_playerLock);
        for (auto it = m_players.begin(); it != m_players.end();)
        {
            if ((*it)->IsDead())
            {
                it = m_players.erase(it);
                continue;
            }

            ++it;
        }
    }
    
    return m_quit == false;
}