#include "pch.h"

#include "MCServer.h"
#include "MCPlayer.h"

#include "Packets/IPacket.h"

#include <chrono>

#include "IGlobalEnvironment.h"
#include "ITCPServer.h"

// TODO server logic

bool CMCServer::Init()
{
    OPTICK_EVENT();

    IGlobalEnvironment::Get()->GetNetwork()->RegisterConnectionCallback(this, [this](IConnectionPtr pConnection) {
        std::lock_guard lock(m_playerLock);
        const std::shared_ptr<CMCPlayer> pPlayer = m_players.emplace_back(std::make_shared<CMCPlayer>(pConnection));
        IGlobalEnvironment::Get()->GetNetwork()->RegisterPacketHandler(pPlayer);
    });
    
    return true;
}

bool CMCServer::Run()
{
    OPTICK_EVENT();

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