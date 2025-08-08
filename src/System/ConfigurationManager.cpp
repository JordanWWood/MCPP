#include "pch.h"
#include "ConfigurationManager.h"

CConfigurationManager::CConfigurationManager(std::string&& configPath)
    : m_configPath(std::move(configPath))
{
}

CConfigurationManager::~CConfigurationManager()
{
    for (auto& kv : m_registeredConfigs) {
        delete kv.second;
        m_registeredConfigs.erase(kv.first);
    }
}

bool CConfigurationManager::Init()
{
    return LoadConfigOrGenerateDefault();
}

void CConfigurationManager::RegisterConfigGroup(IConfigGroup* group)
{
    const auto& pair = m_registeredConfigs.try_emplace(group->m_typeIndex, group);
    if(!pair.second)
    {
        MCLog::error("Config group with type index {} already registered", group->m_typeIndex.name());
        return;
    }

    auto getOrCreateTable = [](toml::table& parent, const std::string& key) -> toml::table&
        {
            if (toml::node* node = parent.get(key); node && node->is_table())
                return *node->as_table();
            return *parent.insert_or_assign(key, toml::table{}).first->second.as_table();
        };

    toml::table& groupTable = group->m_namespaceName.empty()
        ? getOrCreateTable(m_configTable, group->m_groupName)
        : getOrCreateTable(getOrCreateTable(m_configTable, group->m_namespaceName), group->m_groupName);

    auto insertDefault = [&groupTable](IConfigValue* mapping) {
        if (mapping->GetType() == typeid(int))
            groupTable.insert_or_assign(mapping->GetName(), static_cast<SConfigValue<int>*>(mapping)->m_value);
        else if (mapping->GetType() == typeid(std::string))
            groupTable.insert_or_assign(mapping->GetName(), static_cast<SConfigValue<std::string>*>(mapping)->m_value);
        else if (mapping->GetType() == typeid(bool))
            groupTable.insert_or_assign(mapping->GetName(), static_cast<SConfigValue<bool>*>(mapping)->m_value);
        else if (mapping->GetType() == typeid(float))
            groupTable.insert_or_assign(mapping->GetName(), static_cast<SConfigValue<float>*>(mapping)->m_value);
        else if (mapping->GetType() == typeid(double))
            groupTable.insert_or_assign(mapping->GetName(), static_cast<SConfigValue<double>*>(mapping)->m_value);
        else
            MCLog::warn("Unsupported config value type for {}", mapping->GetName());
    };

    auto assignFromNode = [group](IConfigValue* mapping, toml::node& node) {
        if (mapping->GetType() == typeid(int))
        {
            if (auto intValue = node.as_integer())
            {
                std::lock_guard<std::mutex> lock(group->m_mutex);
                static_cast<SConfigValue<int>*>(mapping)->m_value = intValue->get();
            }
            else
            {
                MCLog::error("Config value {} in group {} in namespace {} is not an integer", mapping->GetName(), group->m_groupName, group->m_namespaceName);
            }
        }
        else if (mapping->GetType() == typeid(std::string))
        {
            if (auto strValue = node.as_string())
            {
                std::lock_guard<std::mutex> lock(group->m_mutex);
                static_cast<SConfigValue<std::string>*>(mapping)->m_value = strValue->get();
            }
            else
            {
                MCLog::error("Config value {} in group {} in namespace {} is not a string", mapping->GetName(), group->m_groupName, group->m_namespaceName);
            }
        }
        else if (mapping->GetType() == typeid(bool))
        {
            if (auto boolValue = node.as_boolean())
            {
                std::lock_guard<std::mutex> lock(group->m_mutex);
                static_cast<SConfigValue<bool>*>(mapping)->m_value = boolValue->get();
            }
            else
            {
                MCLog::error("Config value {} in group {} in namespace {} is not a boolean", mapping->GetName(), group->m_groupName, group->m_namespaceName);
            }
        }
        else if (mapping->GetType() == typeid(float))
        {
            if (auto floatValue = node.as_floating_point())
            {
                std::lock_guard<std::mutex> lock(group->m_mutex);
                static_cast<SConfigValue<float>*>(mapping)->m_value = static_cast<float>(floatValue->get());
            }
            else
            {
                MCLog::error("Config value {} in group {} in namespace {} is not a float", mapping->GetName(), group->m_groupName, group->m_namespaceName);
            }
        }
        else if (mapping->GetType() == typeid(double))
        {
            if (auto doubleValue = node.as_floating_point())
            {
                std::lock_guard<std::mutex> lock(group->m_mutex);
                static_cast<SConfigValue<double>*>(mapping)->m_value = doubleValue->get();
            }
            else
            {
                MCLog::error("Config value {} in group {} in namespace {} is not a double", mapping->GetName(), group->m_groupName, group->m_namespaceName);
            }
        }
        else
        {
            MCLog::warn("Unsupported config value type for {}", mapping->GetName());
        }
    };

    bool configChanged = false;
    for (IConfigValue* mapping : group->m_typeMapping)
    {
        if (!mapping)
        {
            MCLog::error("Config value mapping is null for group {} in namespace {}", group->m_groupName, group->m_namespaceName);
            continue;
        }

        toml::node* valueNode = groupTable.get(mapping->GetName());
        if (!valueNode)
        {
            insertDefault(mapping);
            configChanged = true;
            continue;
        }

        assignFromNode(mapping, *valueNode);
    }

    if (configChanged)
    {
        MCLog::info("Wrote new defaults to the config file");

        std::ofstream out(m_configPath);
        out << m_configTable;
    }

    MCLog::info("Registered config group {} in namespace {}", group->m_groupName, group->m_namespaceName);
}

IConfigGroup* CConfigurationManager::GetConfigGroup(const std::type_index& typeIndex)
{
    auto it = m_registeredConfigs.find(typeIndex);
    if(it != m_registeredConfigs.end())
        return it->second;

    MCLog::error("Config group with type index {} not found", typeIndex.name());
    return nullptr;
}

bool CConfigurationManager::LoadConfigOrGenerateDefault()
{
    const std::ifstream fileStream(m_configPath);
    if(fileStream.good())
        return LoadConfig();

    MCLog::info("Could not find config. Generating default config {}", m_configPath);
    
    std::ofstream out(m_configPath);
    out << "";
    out.close();
    
    return LoadConfig();
}

bool CConfigurationManager::LoadConfig()
{
    toml::parse_result result = toml::parse_file(m_configPath);
    if(!result)
    {
        MCLog::error("Failed to parse config file {} with error {}", m_configPath, result.error().description());
        return false;
    }

    m_configTable = result.table();

    for (auto [_, v] : m_registeredConfigs)
    {
        bool hasNamespace = !v->m_namespaceName.empty();
        toml::node_view startingView = m_configTable[hasNamespace ? v->m_namespaceName : v->m_groupName];
        if (hasNamespace)
            startingView = startingView[v->m_groupName];

        for (IConfigValue* mapping : v->m_typeMapping)
        {
            if (!mapping)
            {
                MCLog::error("Config value mapping is null for group {} in namespace {}", v->m_groupName, v->m_namespaceName);
                continue;
            }
            toml::node_view valueView = startingView[mapping->GetName()];
            if (!valueView)
            {
                // Its totally fine if a config value is not present in the config file.
                // We'll just use the default value defined in the config group
                MCLog::debug("Config value {} not found in config for group {} in namespace {}", mapping->GetName(), v->m_groupName, v->m_namespaceName);
                continue;
            }

            if (mapping->GetType() == typeid(int)) {
                if (auto intValue = valueView.as_integer())
                {
                    std::lock_guard<std::mutex> lock(v->m_mutex);
                    static_cast<SConfigValue<int>*>(mapping)->m_value = intValue->get();
                }
                else
                {
                    MCLog::error("Config value {} in group {} in namespace {} is not an integer", mapping->GetName(), v->m_groupName, v->m_namespaceName);
                    return false;
                }
            }
            else if (mapping->GetType() == typeid(std::string)) {
                if (auto strValue = valueView.as_string())
                {
                    std::lock_guard<std::mutex> lock(v->m_mutex);
                    static_cast<SConfigValue<std::string>*>(mapping)->m_value = strValue->get();
                }
                else
                {
                    MCLog::error("Config value {} in group {} in namespace {} is not a string", mapping->GetName(), v->m_groupName, v->m_namespaceName);
                    return false;
                }
            }
            else if (mapping->GetType() == typeid(bool)) {
                if (auto boolValue = valueView.as_boolean())
                {
                    std::lock_guard<std::mutex> lock(v->m_mutex);
                    static_cast<SConfigValue<bool>*>(mapping)->m_value = boolValue->get();
                }
                else
                {
                    MCLog::error("Config value {} in group {} in namespace {} is not a boolean", mapping->GetName(), v->m_groupName, v->m_namespaceName);
                    return false;
                }
            }
            else if (mapping->GetType() == typeid(float)) {
                if (auto floatValue = valueView.as_floating_point())
                {
                    std::lock_guard<std::mutex> lock(v->m_mutex);
                    static_cast<SConfigValue<float>*>(mapping)->m_value = floatValue->get();
                }
                else
                {
                    MCLog::error("Config value {} in group {} in namespace {} is not a float", mapping->GetName(), v->m_groupName, v->m_namespaceName);
                    return false;
                }
            }
            else if (mapping->GetType() == typeid(double)) {
                if (auto doubleValue = valueView.as_floating_point())
                {
                    std::lock_guard<std::mutex> lock(v->m_mutex);
                    static_cast<SConfigValue<double>*>(mapping)->m_value = doubleValue->get();
                }
                else
                {
                    MCLog::error("Config value {} in group {} in namespace {} is not a double", mapping->GetName(), v->m_groupName, v->m_namespaceName);
                    return false;
                }
            }
            else { 
                MCLog::warn("Unsupported config value type for {}", mapping->GetName());
            }

        }
    }
    
    return true;
}
