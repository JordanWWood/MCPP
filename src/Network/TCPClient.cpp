#include "pch.h"
#include "TCPClient.h"

void CTCPClient::Start()
{
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints = {};

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(address.c_str(), DEFAULTPORT, &hints, &result);
    
}
