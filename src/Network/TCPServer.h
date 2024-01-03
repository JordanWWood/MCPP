#pragma once

#include <memory>

#include "ITCPServer.h"
#include "SocketUtils.h"

// TODO tidy up includes
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <nunistd.h>
#endif

struct IRSAKeyPair;
struct IConnection;
using IConnectionPtr = std::shared_ptr<IConnection>;

class CTCPServer final : public ITCPServer
{
public:
    CTCPServer(const uint16_t port);
    ~CTCPServer();

    //////////////////////////////////////////////////
    // ITCPServer
    virtual bool Listen() override;

    virtual IConnectionPtr AcceptConnection() const override;
    virtual bool IsSocketClosed() const override { return m_listenSocketState == ESocketState::eSS_CLOSED; }

    virtual std::shared_ptr<IRSAKeyPair> GenerateRSAKeyPair() override;
    // ~ITCPServer
    //////////////////////////////////////////////////
private:
    SOCKET m_listenSocket { INVALID_SOCKET };
    ESocketState m_listenSocketState { ESocketState::eSS_UNINITIALISED };

    const uint16_t m_port{ 25565 };
};
