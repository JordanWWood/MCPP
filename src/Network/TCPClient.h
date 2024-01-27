#pragma once

#include "ITCPClient.h"

//TODO this is temp
#define DEFAULTPORT "25566"

class CTCPClient : public ITCPClient
{
public:
    void Start();
    
private:
    // TODO set this based on the server we want to root a player to
    std::string address = "127.0.0.1";
    SOCKET m_socket { INVALID_SOCKET };
};
