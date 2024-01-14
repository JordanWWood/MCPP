#pragma once

#include "IGlobalEnvironment.h"
#include "INetwork.h"

class CGlobalEnvironment : public IGlobalEnvironment
{
public:
    CGlobalEnvironment();

    // Systems
    INetwork* GetNetwork() const override { return m_pNetwork; }
    void SetNetwork(INetwork* pNetwork) { m_pNetwork = pNetwork; }
    ICurlProcessor* GetCurl() const override { return m_pCurlProcessor; }
    void SetCurl(ICurlProcessor* pCurl) { m_pCurlProcessor = pCurl; }

    // Settings
    bool IsOnline() const override { return isOnline; }

private:
    INetwork* m_pNetwork;
    ICurlProcessor* m_pCurlProcessor;

    bool isOnline = true;
};
