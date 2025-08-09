#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

#include <map>
#include <typeindex>
#include <mutex>
#include <type_traits>
#include <utility>

// Forward declarations for ConfigStructTraits are provided later.

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

#define CONFIG_GROUP_BEGIN(structName, configNamespace, group)                               \
    namespace config { namespace configNamespace { struct group {}; } }                      \
    class structName : public IConfigGroup {                                                 \
    public:                                                                                  \
        structName()                                                                         \
            : IConfigGroup(typeid(config::configNamespace::group), #configNamespace, #group) \
        {}

#define CONFIG_GROUP_BEGIN_NO_NAMESPACE(structName, group) \
    namespace config { struct group {}; }                      \
    class structName : public IConfigGroup {                                                 \
    public:                                                                                  \
        structName()                                                                         \
            : IConfigGroup(typeid(config::group), "", #group) \
        {}

#define CONFIG_GROUP_MEMBER(type, variable, defaultValue)           \
    private:                                                        \
    SConfigValue<type> variable { #variable, defaultValue, *this }; \
    public:                                                         \
    type Get##variable() const {                                    \
        std::lock_guard<std::mutex> lock(m_mutex);                  \
        return variable.m_value;                                    \
    }                                                               \

#define CONFIG_GROUP_END() \
    };

// This macro is used to register the config group with the global environment.
#define REGISTER_CONFIG_GROUP(structName)                                                                 \
    /* We can use a struct with a constructor to run some code for us in the context the macro is used */ \
    struct Register##structName {                                                                         \
        Register##structName() {                                                                          \
            IGlobalEnvironment::Get().GetConfigManager().lock()->RegisterConfigGroup(new structName());   \
        }                                                                                                 \
    };                                                                                                    \
    volatile Register##structName structName##Reg;

template<typename T>
struct ConfigStructTraits;

// Macros to mark user-defined structs so their members can be visited by the
// configuration system. Example usage:
//
// struct SServer { std::string address; int port; };
// CONFIG_STRUCT_BEGIN(SServer)
//     CONFIG_STRUCT_MEMBER(address)
//     CONFIG_STRUCT_MEMBER(port)
// CONFIG_STRUCT_END()
#define CONFIG_STRUCT_BEGIN(structName)                                           \
    template<> struct ConfigStructTraits<structName> {                            \
        template<typename F> static void for_each(structName& obj, F&& f) {

#define CONFIG_STRUCT_MEMBER(member)                                             \
            f(#member, obj.member);

#define CONFIG_STRUCT_END()                                                      \
        }                                                                        \
    };

#define CONCAT(...) __VA_ARGS__

struct IConfigGroup;

struct IConfigValueVisitor
{
    virtual ~IConfigValueVisitor() = default;

    virtual bool IsReading() const = 0;

    virtual void OnInt(const char* name, int& value) = 0;
    virtual void OnString(const char* name, std::string& value) = 0;
    virtual void OnBool(const char* name, bool& value) = 0;
    virtual void OnFloat(const char* name, float& value) = 0;
    virtual void OnDouble(const char* name, double& value) = 0;

    virtual void OnArrayBegin(const char* name, size_t& size) = 0;
    virtual void OnArrayEnd() = 0;

    virtual void OnStructBegin(const char* name) = 0;
    virtual void OnStructEnd() = 0;
};

namespace config_detail
{
    template<typename T>
    struct is_vector : std::false_type {};
    template<typename T, typename A>
    struct is_vector<std::vector<T, A>> : std::true_type {};

    template<typename T, typename = void>
    struct is_config_struct : std::false_type {};
    struct dummy_visitor
    {
        template<typename U>
        void operator()(const char*, U&) const {}
    };

    template<typename T>
    struct is_config_struct < T, std::void_t<decltype(ConfigStructTraits<T>::for_each(std::declval<T&>(), dummy_visitor{})) >> : std::true_type {};
    
    template<typename T>
    inline constexpr bool is_config_struct_v = is_config_struct<T>::value;

    template<typename T>
    void Visit(IConfigValueVisitor& visitor, const char* name, T& value)
    {
        if constexpr (is_vector<T>::value)
        {
            size_t size = value.size();
            visitor.OnArrayBegin(name, size);
            if (visitor.IsReading())
                value.resize(size);
            for (size_t i = 0; i < size; ++i)
                Visit(visitor, nullptr, value[i]);
            visitor.OnArrayEnd();
        }
        else if constexpr (is_config_struct_v<T>)
        {
            visitor.OnStructBegin(name);
            ConfigStructTraits<T>::for_each(value, [&](const char* fieldName, auto& field)
                {
                    Visit(visitor, fieldName, field);
                });
            visitor.OnStructEnd();
        }
        else if constexpr (std::is_same_v<T, int>)
            visitor.OnInt(name, value);
        else if constexpr (std::is_same_v<T, std::string>)
            visitor.OnString(name, value);
        else if constexpr (std::is_same_v<T, bool>)
            visitor.OnBool(name, value);
        else if constexpr (std::is_same_v<T, float>)
            visitor.OnFloat(name, value);
        else if constexpr (std::is_same_v<T, double>)
            visitor.OnDouble(name, value);
    }
}

struct IConfigValue {
    virtual ~IConfigValue() = default;

    virtual std::string GetName() const = 0;
    virtual const std::type_info& GetType() const = 0;
    virtual void* GetValue() = 0;

    virtual void Accept(IConfigValueVisitor& visitor) = 0;
};

struct IConfigGroup
{
    IConfigGroup(std::type_index typeIndex, std::string namespaceName, std::string groupName)
        : m_namespaceName(std::move(namespaceName))
        , m_groupName(std::move(groupName))
        , m_typeIndex(typeIndex)
    {
    }

    virtual ~IConfigGroup() = default;

    std::string m_namespaceName;
    std::string m_groupName;

    std::type_index m_typeIndex;
    std::vector<IConfigValue*> m_typeMapping{ };
    mutable std::mutex m_mutex;
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

    void Accept(IConfigValueVisitor& visitor) override
    {
        config_detail::Visit(visitor, m_variableName.c_str(), m_value);
    }

    const std::string m_variableName;
    IConfigGroup& m_group;
    T m_value;
};

struct IConfigurationManager
{
    virtual ~IConfigurationManager() = default;

    virtual void RegisterConfigGroup(IConfigGroup* group) = 0;

    template<typename T>
    IConfigGroup* GetConfigGroup() {
        return GetConfigGroup(typeid(T));
    }

protected:
    virtual IConfigGroup* GetConfigGroup(const std::type_index& typeIndex) = 0;
};
