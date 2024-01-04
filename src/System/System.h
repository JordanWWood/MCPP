#pragma once

#include "GlobalEnvironment.h"
#include "IServer.h"
#include "INetwork.h"

class CSystem
{
public:
    bool Init();
    bool Run();

private:
    std::unique_ptr<IServer> m_pServer{ nullptr };
    std::unique_ptr<INetwork> m_pNetwork{ nullptr };
    CGlobalEnvironment m_globalEnvironment;
};
