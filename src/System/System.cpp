#include "pch.h"

#include "System.h"
#include "MCServer.h"
#include "Network.h"

#define MAIN_THREAD_UPDATE_RATE 20
using TMainThreadFrame = std::chrono::duration<int64_t, std::ratio<1, MAIN_THREAD_UPDATE_RATE>>;

bool CSystem::Init()
{
    m_pNetwork = std::make_unique<CNetwork>();
    m_globalEnvironment.SetNetwork(m_pNetwork.get());

    m_pServer = std::make_unique<CMCServer>();
    return m_pServer->Init();
}

bool CSystem::Run()
{
    auto nextFrame = std::chrono::high_resolution_clock::now() + TMainThreadFrame{1};
    
    while (true)
    {
        OPTICK_FRAME("Main Thread");
        
        // TODO Network Main Thread Start
        
        // Main Thread Update
        m_pServer->Run();
        
        // TODO Network Main Thread End

        // Frame synchronisation
        OPTICK_EVENT("Sync Frame");
        OPTICK_TAG("FPS", MAIN_THREAD_UPDATE_RATE);
        std::this_thread::sleep_until(nextFrame);
        nextFrame += TMainThreadFrame{1};
    }
}
