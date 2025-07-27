#pragma once
#include "IConfiguration.h"

#include <typeinfo>

struct ConfigElement {
    std::string m_name;

    void* m_value;
};

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

    std::vector<SServer> m_servers;

    std::unordered_map<std::string, ConfigElement> m_configValues;

    bool m_isOnline{ true };
    uint16_t m_hostPort{ 0 };
    std::string m_configPath;
};
