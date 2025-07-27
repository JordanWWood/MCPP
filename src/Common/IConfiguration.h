#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include <map>
#include <typeindex>

// Config groups represent a group of configurable variables that are used by a given area of code.
// For example the following usage
// CONFIG_GROUP_BEGIN(SSocketConfig, network, socket)
// CONFIG_GROUP_MEMBER(std::string, ip, "0.0.0.0")
// CONFIG_GROUP_MEMBER(uint16, port, 25565)
// CONFIG_GROUP_END()
//
// Will produce the following TOML
// [network]
// [network.socket]
// ip="0.0.0.0"
// port=25565
//
// and can be retrieved by passing the namespace and category to GetConfig where a specialization will return
// the originally provided type
// SSocketConfig config = CConfigManager::Get().GetConfig<config::network::socket>();

#define CONFIG_GROUP_BEGIN(structName, configNamespace, group)                                  \
    namespace config { namespace configNamespace { struct group {}; } }                         \
    struct structName : public IConfigGroup {                                                   \
        structName()                                                                            \
            : IConfigGroup(typeid(config::configNamespace::group), #configNamespace, #group)    \
        {}

#define CONFIG_GROUP_MEMBER(type, variable, defaultValue)           \
    SConfigValue<type> variable { #variable, defaultValue, *this }; \

#define CONFIG_GTROUP_END() \
    };

// This macro is used to register the config group with the global environment.
#define REGISTER_CONFIG_GROUP(structName) \
    /* We can use a struct with a constructor to run some code for us in the context the macro is used */   \
    struct Register##structName {                                                                           \
        Register##structName() {                                                                            \
            IGlobalEnvironment::Get()->GetConfigManager()->RegisterConfigGroup(new structName());)          \
        }                                                                                                   \
    };                                                                                                      \
    Register##structName structName##Reg();

class IConfigGroup
{
    IConfigGroup(std::type_index typeIndex, std::string namespaceName, std::string groupName)
        : m_namespaceName(std::move(namespaceName))
        , m_groupName(std::move(groupName))
        , m_typeIndex(typeIndex)
    {
    }

    std::string m_namespaceName;
    std::string m_groupName;

    std::type_index m_typeIndex;
    std::vector<IConfigValue*> m_typeMapping;
};

struct IConfigValue {
    virtual ~IConfigValue() = default;

    virtual std::string GetName() const = 0;
    virtual const std::type_info& GetType() const = 0;
    virtual void* GetValue() = 0;
};

template<typename T>
struct SConfigValue : public IConfigValue
{
    SConfigValue(std::string variableName, T value, IConfigGroup& group)
        : m_group(group)
        , m_value(value)
        , m_variableName(variableName)
    {
        m_group.m_typeMapping.push_back(this);
    }

    std::string GetName() const override { return m_variableName; }
    void* GetValue() override { return &m_value; }

    const std::type_info& GetType() const override { return typeid(T); }

    const std::string m_variableName;
    IConfigGroup& m_group;
    T m_value;
};

struct IConfigurationManager
{
    virtual ~IConfigurationManager() = default;
};
