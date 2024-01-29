#pragma once
#include "IConfiguration.h"

class CConfigurationManager : public IConfiguration
{
public:
    CConfigurationManager(std::string&& configPath);

    bool Init();
    
    /////////////////////////////////////////////////////////////////////
    // IConfiguration
    bool IsOnline() const override { return m_isOnline; };
    const std::vector<SServer>& GetPredefinedServers() const override { return m_servers; }
    uint16_t GetHostPort() const override { return m_hostPort; }
    // ~IConfiguration
    /////////////////////////////////////////////////////////////////////
    
private:
    bool LoadConfigOrGenerateDefault();
    bool LoadConfig();

    bool m_isOnline;
    uint16_t m_hostPort;
    std::vector<SServer> m_servers;

    std::string m_configPath;
};
