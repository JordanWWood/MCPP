#pragma once

#include "GlobalEnvironment.h"
#include "IServer.h"
#include "INetwork.h"
#include "ICurlProcessor.h"

class CSystem
{
public:
    bool Init();
    bool Run();

private:
    std::shared_ptr<IServer> m_pServer{ nullptr };
    std::shared_ptr<INetwork> m_pNetwork{ nullptr };
    std::shared_ptr<ICurlProcessor> m_pCurlProcessor{ nullptr };
    std::shared_ptr<IConfiguration> m_pConfiguration{ nullptr };
    CGlobalEnvironment m_globalEnvironment;
};
