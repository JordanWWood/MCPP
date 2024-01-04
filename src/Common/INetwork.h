#pragma once

#include "ITCPServer.h"

struct INetwork
{
    virtual ~INetwork() = default;
    
    virtual ITCPServer* GetTCPServer() = 0;
};
