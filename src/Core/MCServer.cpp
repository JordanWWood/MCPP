#include "MCServer.h"

#include <chrono>
#include <thread>

#define UPDATE_RATE 20

CMCServer::CMCServer()
    : m_pTcpServer(std::make_unique<CTCPServer>(25565))
{
}

bool CMCServer::Init(uint16_t port)
{
    return m_pTcpServer->Listen();
}

bool CMCServer::Run()
{
    auto start = std::chrono::high_resolution_clock::now();

    // TODO we should reestablish the listen socket if it closes. For now the application just exits
    if(m_pTcpServer->IsSocketClosed())
        return false;
    
    // Network update
    // 1 Accept new connections
    if(IClientPtr pClient = m_pTcpServer->AcceptConnection())
        m_clients.push_back(pClient);

    // 2 Process packets
    // TODO move to own threads
    for (std::vector<IClientPtr>::iterator it = m_clients.begin(); it != m_clients.end();)
    {
        IClientPtr& client = *it;
        bool result = client->RecvPackets();

        if (client->IsSocketClosed())
        {
            it = m_clients.erase(it); // This client is no longer connected. Remove it
            continue;
        }

        ++it;
    }

    

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = end-start;
    
    if (elapsedTime < std::chrono::milliseconds(1000 / UPDATE_RATE))
        std::this_thread::sleep_until(start + std::chrono::milliseconds(33));

    return true;
}
