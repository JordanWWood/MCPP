#pragma once
#include "IConfigurationManager.h"

#include "../../vendor/tomlplusplus/include/toml++/toml.hpp"
#include <unordered_map>
#include <typeindex>
#include <string>

class CConfigurationManager : public IConfigurationManager
{
public:
    CConfigurationManager(std::string&& configPath);
    ~CConfigurationManager();

    bool Init();

    /////////////////////////////////////////////////////////////////////
    // IConfiguration
    void RegisterConfigGroup(IConfigGroup* group) override;
    IConfigGroup* GetConfigGroup(const std::type_index& typeIndex) override;
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