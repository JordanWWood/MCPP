#pragma once

#include <cstdint>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "MCPlayer.h"

struct IRSAKeyPair;
struct ITCPServer;

class CMCServer
{
public:
    CMCServer(std::unique_ptr<ITCPServer> tcpServer);
    
    bool Init();
    bool Run();

    void NetworkRun();
private:
    std::unique_ptr<ITCPServer> m_pTcpServer;
    std::vector<CMCPlayer> m_players;

    std::atomic<bool> m_quit;
    std::thread m_networkThread;
    std::mutex m_networkLock;

    std::shared_ptr<IRSAKeyPair> m_pKeyPair;
};
