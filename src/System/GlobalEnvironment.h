#pragma once

#include "IGlobalEnvironment.h"
#include "INetwork.h"

class CGlobalEnvironment : public IGlobalEnvironment
{
public:
    CGlobalEnvironment();
    
    INetwork* GetNetwork() const override { return m_pNetwork; }
    void SetNetwork(INetwork* pNetwork) { m_pNetwork = pNetwork; }
    ICurlProcessor* GetCurl() const override { return m_pCurlProcessor; }
    void SetCurl(ICurlProcessor* pCurl) { m_pCurlProcessor = pCurl; }

private:
    INetwork* m_pNetwork;
    ICurlProcessor* m_pCurlProcessor;
};
