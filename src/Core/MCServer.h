#pragma once

#include <cstdint>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "IServer.h"
#include "MCPlayer.h"

struct IRSAKeyPair;
struct ITCPServer;

class CMCServer : public IServer
{
public:
    virtual bool Init() override;
    virtual bool Run() override;

    void NetworkRun();
private:
    std::vector<CMCPlayer> m_players;

    std::atomic<bool> m_quit;
    std::thread m_networkThread;
    std::mutex m_networkLock;

    std::shared_ptr<IRSAKeyPair> m_pKeyPair;
};
