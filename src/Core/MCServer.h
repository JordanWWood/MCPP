#pragma once

#include <cstdint>
#include <memory>

#include "Networking/TCPServer.h"

class CMCServer
{
public:
    CMCServer();
    
    bool Init(uint16_t port);
    bool Run();
private:
    std::unique_ptr<CTCPServer> m_pTcpServer;
    std::vector<IClientPtr> m_clients;
};
