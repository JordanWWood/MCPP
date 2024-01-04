#include "pch.h"
#include "GlobalEnvironment.h"

CGlobalEnvironment::CGlobalEnvironment()
{
    if (m_sGlobalEnvironment != nullptr)
    {
        MCLog::critical("Global environment should always be a singleton. One seems to already exist");
        return;
    }
        
    m_sGlobalEnvironment = this;
}