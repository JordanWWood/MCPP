#pragma once

#include "ICurlProcessor.h"
#include "INetwork.h"

struct IGlobalEnvironment
{
    virtual ~IGlobalEnvironment() = default;
    virtual INetwork* GetNetwork() const = 0;
    virtual ICurlProcessor* GetCurl() const = 0;
    
    static IGlobalEnvironment* Get() { return m_sGlobalEnvironment; }

protected:
    static IGlobalEnvironment* m_sGlobalEnvironment;
};