#pragma once
#include "INetwork.h"
#include "TCPServer.h"

class CNetwork final : public INetwork
{
public:
    CNetwork();

    ITCPServer* GetTCPServer() override { return &m_tcpServer; }
private:
    CTCPServer m_tcpServer;
};
