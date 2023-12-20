#pragma once

#include "Networking/SocketUtils.h"

class IClient
{
public:
    virtual bool RecvPackets() = 0;
    virtual bool IsSocketClosed() const = 0;
};
