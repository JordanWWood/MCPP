#include "pch.h"

#include "ProxyServer.h"
#include "MCPlayer.h"

#include "IGlobalEnvironment.h"

bool CProxyServer::Init()
{
    MCPP_PROFILE_SCOPE()

    IGlobalEnvironment::Get()->GetNetwork().lock()->RegisterConnectionCallback(this, [this](const IConnectionPtr& pConnection) {
        std::lock_guard lock(m_playerLock);
        const std::shared_ptr<CMCPlayer> pPlayer = m_players.emplace_back(std::make_shared<CMCPlayer>(pConnection));
        IGlobalEnvironment::Get()->GetNetwork().lock()->RegisterPacketHandler(pPlayer);
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