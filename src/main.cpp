#include <iostream>

#include "Networking/TCPServer.h"

int main(int argc, char** argv)
{
    CTCPServer server(25565);
    server.Listen();

    // Network loop
    while(true)
    {
        server.AcceptConnection();
        server.RecvPackets();
    }
        
    return 0;
}
