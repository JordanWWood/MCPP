#include "pch.h"

#include "MCServer.h"
#include "MCPlayer.h"

#include "Packets/IPacket.h"

#include <chrono>

#include "IGlobalEnvironment.h"
#include "ITCPServer.h"

#define NETWORK_THREAD_UPDATE_RATE 120

using TNetworkThreadFrame = std::chrono::duration<int64_t, std::ratio<1, NETWORK_THREAD_UPDATE_RATE>>;

#define THREAD_UPDATE_BEGIN(frame) auto nextFrame = std::chrono::system_clock::now() + frame{1}
#define THREAD_UPDATE_END() std::this_thread::sleep_until(nextFrame)

static void NetworkThread(CMCServer* mcServer)
{
    OPTICK_THREAD("Network Thread");

    mcServer->NetworkRun();
}

bool CMCServer::Init()
{
    OPTICK_EVENT();

    MCLog::info("Initialising MC Server");
    m_networkThread = std::thread(NetworkThread, this);

    MCLog::info("Generating RSA key pair");
    m_pKeyPair = IGlobalEnvironment::Get()->GetNetwork()->GetTCPServer()->GenerateRSAKeyPair();
    
    return true;
}

bool CMCServer::Run()
{
    OPTICK_EVENT();
    
    return m_quit == false;
}

// TODO move this into Network.cpp
void CMCServer::NetworkRun()
{
    m_quit = !IGlobalEnvironment::Get()->GetNetwork()->GetTCPServer()->Listen();

    while (!m_quit)
    {
        THREAD_UPDATE_BEGIN(TNetworkThreadFrame);
        
        {
            OPTICK_EVENT("Network Update");

            // TODO we should reestablish the listen socket if it closes. For now the application just exits
            if (IGlobalEnvironment::Get()->GetNetwork()->GetTCPServer()->IsSocketClosed())
                break;

            {
                std::lock_guard lock(m_networkLock);

                // Network update
                // 1 Accept new connections
                if (IConnectionPtr pConnection = IGlobalEnvironment::Get()->GetNetwork()->GetTCPServer()->AcceptConnection())
                    m_players.emplace_back(pConnection, m_pKeyPair);

                // 2 Update current connections
                // TODO move to own threads
                for (std::vector<CMCPlayer>::iterator it = m_players.begin(); it != m_players.end();)
                {
                    CMCPlayer& player = *it;
                    player.NetworkTick();

                    if (player.IsDead())
                    {
                        if (player.GetCurrentState() >= EClientState::eCS_Login)
                            MCLog::info("Client has disconnected. Username[{}] State[{}]", player.GetUsername(), static_cast<uint32_t>(player.GetCurrentState()));
                        else
                            MCLog::info("Server list ping disconnected. State[{}]", static_cast<uint32_t>(player.GetCurrentState()));

                        it = m_players.erase(it); // This client is no longer connected. Remove it
                        continue;
                    }

                    ++it;
                }
            }
        }

        {
            OPTICK_EVENT("Sleep");
            THREAD_UPDATE_END();
        }
    }

    m_quit = true;
}