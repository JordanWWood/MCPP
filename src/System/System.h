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
    std::unique_ptr<IServer> m_pServer{ nullptr };
    std::unique_ptr<INetwork> m_pNetwork{ nullptr };
    std::unique_ptr<ICurlProcessor> m_pCurlProcessor{ nullptr };
    CGlobalEnvironment m_globalEnvironment;
};
