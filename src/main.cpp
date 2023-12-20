
#include "Core/MCServer.h"

int main(int argc, char** argv)
{
    CMCServer server;

    if (server.Init(25565))
    {
        while(server.Run()) {}
    }
    
    return 0;
}
