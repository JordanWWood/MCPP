#pragma once

#include <cstdint>
#include <memory>
#include <atomic>
#include <thread>

#include "Networking/TCPServer.h"

class CMCServer
{
public:
    CMCServer(uint16_t port);
    
    bool Init();
    bool Run();

    void NetworkRun();
private:
    std::unique_ptr<CTCPServer> m_pTcpServer;
    std::vector<IClientPtr> m_clients;

    std::atomic<bool> m_quit;
    std::thread m_networkThread;
};
