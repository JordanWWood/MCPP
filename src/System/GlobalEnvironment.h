#pragma once

#include "IGlobalEnvironment.h"
#include "INetwork.h"

class CGlobalEnvironment : public IGlobalEnvironment
{
public:
    CGlobalEnvironment();
    
    INetwork* GetNetwork() const override { return m_pNetwork; }
    void SetNetwork(INetwork* pNetwork) { m_pNetwork = pNetwork; }

private:
    INetwork* m_pNetwork;
};
