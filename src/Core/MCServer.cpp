#include "pch.h"

#include "MCServer.h"
#include "MCPlayer.h"

#include "Common/Packets/IPacket.h"

#include <chrono>

#define MAIN_THREAD_UPDATE_RATE 20
#define NETWORK_THREAD_UPDATE_RATE 120

using TMainThreadFrame = std::chrono::duration<int64_t, std::ratio<1, MAIN_THREAD_UPDATE_RATE>>;
using TNetworkThreadFrame = std::chrono::duration<int64_t, std::ratio<1, NETWORK_THREAD_UPDATE_RATE>>;

#define THREAD_UPDATE_BEGIN(frame) auto nextFrame = std::chrono::system_clock::now() + frame{1}
#define THREAD_UPDATE_END() std::this_thread::sleep_until(nextFrame)

static void NetworkThread(CMCServer* mcServer)
{
    mcServer->NetworkRun();
}

CMCServer::CMCServer(uint16_t port)
    : m_pTcpServer(std::make_unique<CTCPServer>(port))
{
}

bool CMCServer::Init()
{
    MCLog::info("Initialising MC Server");
    m_networkThread = std::thread(NetworkThread, this);

    MCLog::info("Generating RSA key pair");
    m_pKeyPair = m_pTcpServer->GenerateRSAKeyPair();
    
    return true;
}

bool CMCServer::Run()
{
    THREAD_UPDATE_BEGIN(TMainThreadFrame);

    

    THREAD_UPDATE_END();

    return m_quit == false;
}

void CMCServer::NetworkRun()
{
    m_quit = !m_pTcpServer->Listen();

    while (!m_quit)
    {
        THREAD_UPDATE_BEGIN(TNetworkThreadFrame);

        // TODO we should reestablish the listen socket if it closes. For now the application just exits
        if (m_pTcpServer->IsSocketClosed())
            break;

        {
            std::lock_guard lock(m_networkLock); 
            
            // Network update
            // 1 Accept new connections
            if (IConnectionPtr pClient = m_pTcpServer->AcceptConnection())
                m_players.emplace_back(pClient, m_pKeyPair);

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

        THREAD_UPDATE_END();
    }

    m_quit = true;
}