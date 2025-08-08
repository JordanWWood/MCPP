#pragma once
#include "IConfigurationManager.h"

#include <typeinfo>

#define TOML_IMPLEMENTATION
#include <toml++/toml.hpp>


class CConfigurationManager : public IConfigurationManager
{
public:
    CConfigurationManager(std::string&& configPath);
    ~CConfigurationManager();

    bool Init();
    
    /////////////////////////////////////////////////////////////////////
    // IConfiguration
    virtual void RegisterConfigGroup(IConfigGroup* group) override;
    virtual IConfigGroup* GetConfigGroup(const std::type_index& typeIndex) override;
    // ~IConfiguration
    /////////////////////////////////////////////////////////////////////

private:
    bool LoadConfigOrGenerateDefault();
    bool LoadConfig();

    using TConfigMap = std::unordered_map<std::type_index, IConfigGroup*>;
    TConfigMap m_registeredConfigs;

    toml::table m_configTable;
    std::string m_configPath;
};
