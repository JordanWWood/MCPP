#include "pch.h"

#include "System.h"

#include "ConfigurationManager.h"
#include "CurlProcessor.h"
#include "ProxyServer.h"
#include "Network.h"

#define MAIN_THREAD_UPDATE_RATE 20
using TMainThreadFrame = std::chrono::duration<int64_t, std::ratio<1, MAIN_THREAD_UPDATE_RATE>>;

// TODO allow this to be passed by commandline
#define CONFIG_FILE "config.toml"

bool CSystem::Init()
{
    // Load the config before we initialise any other systems
    std::shared_ptr<CConfigurationManager> configManager = std::make_shared<CConfigurationManager>(CONFIG_FILE);
    if(!configManager->Init())
        return false;
    m_pConfiguration = configManager;
    m_globalEnvironment.SetConfiguration(configManager);
    
    m_pNetwork = std::make_shared<CNetwork>(m_pConfiguration->GetHostPort());
    m_globalEnvironment.SetNetwork(m_pNetwork);

    m_pCurlProcessor = std::make_shared<CCurlProcessor>();
    m_globalEnvironment.SetCurl(m_pCurlProcessor);

    m_pServer = std::make_unique<CProxyServer>();
    return m_pServer->Init();
}

bool CSystem::Run()
{
    auto nextFrame = std::chrono::high_resolution_clock::now() + TMainThreadFrame{1};
    
    while (true)
    {
        OPTICK_FRAME("Main Thread");
        if(m_pNetwork->HasShutdown())
            return false;
        
        // TODO Network Main Thread Start
        
        // Main Thread Update
        m_pServer->Run();
        
        // TODO Network Main Thread End

        // Frame synchronisation
        MCPP_PROFILE_NAMED_SCOPE("Sync Frame");
        OPTICK_TAG("FPS", MAIN_THREAD_UPDATE_RATE);
        std::this_thread::sleep_until(nextFrame);
        nextFrame += TMainThreadFrame{1};
    }
}
