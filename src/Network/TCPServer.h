#pragma once

#include <memory>

#include "ITCPServer.h"
#include "SocketUtils.h"

struct IRSAKeyPair;
struct IConnection;
using IConnectionPtr = std::shared_ptr<IConnection>;

class CTCPServer final : public ITCPServer
{
public:
    CTCPServer(const uint16_t port);
    ~CTCPServer() override;

    /////////////////////////////////////////////////////////////////////
    // ITCPServer
    virtual bool Listen() override;

    virtual IConnectionPtr AcceptConnection() const override;
    virtual bool IsSocketClosed() const override { return m_listenSocketState == ESocketState::eSS_CLOSED; }
    // ~ITCPServer
    /////////////////////////////////////////////////////////////////////
private:
    UINT_PTR m_listenSocket { INVALID_SOCKET };
    ESocketState m_listenSocketState { ESocketState::eSS_UNINITIALISED };

    const uint16_t m_port{ 25565 };
};
