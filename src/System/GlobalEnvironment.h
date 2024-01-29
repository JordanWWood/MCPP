#pragma once

#include "IConfiguration.h"
#include "IGlobalEnvironment.h"
#include "INetwork.h"

class CGlobalEnvironment : public IGlobalEnvironment
{
public:
    CGlobalEnvironment();

    // Systems
    std::weak_ptr<INetwork> GetNetwork() const override { return m_pNetwork; }
    void SetNetwork(const std::shared_ptr<INetwork>& pNetwork) { m_pNetwork = pNetwork; }
    std::weak_ptr<ICurlProcessor> GetCurl() const override { return m_pCurlProcessor; }
    void SetCurl(const std::shared_ptr<ICurlProcessor>& pCurl) { m_pCurlProcessor = pCurl; }
    std::weak_ptr<IConfiguration> GetConfiguration() const override { return m_pConfiguration; }
    void SetConfiguration(const std::shared_ptr<IConfiguration>& pConfig) { m_pConfiguration = pConfig; }

    // Settings
    bool IsOnline() const override { return isOnline; }

private:
    std::shared_ptr<INetwork> m_pNetwork;
    std::shared_ptr<ICurlProcessor> m_pCurlProcessor;
    std::shared_ptr<IConfiguration> m_pConfiguration;
    
    bool isOnline = true;
};
