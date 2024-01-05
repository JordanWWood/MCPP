#pragma once

#include <cstdint>
#include <memory>
#include <atomic>
#include <mutex>
#include <vector>

#include "IServer.h"
#include "MCPlayer.h"

struct ITCPServer;

class CMCServer : public IServer
{
public:
    virtual bool Init() override;
    virtual bool Run() override;
    
private:
    std::vector<std::shared_ptr<CMCPlayer>> m_players;
    std::mutex m_playerLock;

    std::atomic<bool> m_quit;
};
