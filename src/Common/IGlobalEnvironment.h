#pragma once

#include "ICurlProcessor.h"
#include "INetwork.h"

struct IGlobalEnvironment
{
    virtual ~IGlobalEnvironment() = default;
    static IGlobalEnvironment* Get() { return m_sGlobalEnvironment; }
    
    // Systems
    virtual INetwork* GetNetwork() const = 0;
    virtual ICurlProcessor* GetCurl() const = 0;

    // Settings
    virtual bool IsOnline() const = 0;
    
protected:
    static IGlobalEnvironment* m_sGlobalEnvironment;
};