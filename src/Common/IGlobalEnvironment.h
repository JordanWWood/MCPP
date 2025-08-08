#pragma once

#include "IConfigurationManager.h"
#include "ICurlProcessor.h"
#include "INetwork.h"

struct IGlobalEnvironment
{
    virtual ~IGlobalEnvironment() = default;
    static IGlobalEnvironment& Get() { return *m_sGlobalEnvironment; }
    
    // Systems
    virtual std::weak_ptr<INetwork> GetNetwork() const = 0;
    virtual std::weak_ptr<ICurlProcessor> GetCurl() const = 0;
    virtual std::weak_ptr<IConfigurationManager> GetConfigManager() const = 0;

    // Settings
    virtual bool IsOnline() const = 0;
    
protected:
    static IGlobalEnvironment* m_sGlobalEnvironment;
};