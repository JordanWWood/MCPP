#pragma once

#include <cstdint>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "Networking/TCPServer.h"
#include "MCPlayer.h"

struct IRSAKeyPair;

class CMCServer
{
public:
    CMCServer(uint16_t port);
    
    bool Init();
    bool Run();

    void NetworkRun();
private:
    std::unique_ptr<CTCPServer> m_pTcpServer;
    std::vector<CMCPlayer> m_players;

    std::atomic<bool> m_quit;
    std::thread m_networkThread;
    std::mutex m_networkLock;

    std::shared_ptr<IRSAKeyPair> m_pKeyPair;
};
