#include "pch.h"
#define TOML_IMPLEMENTATION
#include "ConfigurationManager.h"


class DefaultWriter : public IConfigValueVisitor
{
public:
    explicit DefaultWriter(toml::table& root) { m_stack.push_back(&root); }

    bool IsReading() const override { return false; }

    void OnInt(const char* name, int& value) override { insert(name, value); }
    void OnString(const char* name, std::string& value) override { insert(name, value); }
    void OnBool(const char* name, bool& value) override { insert(name, value); }
    void OnFloat(const char* name, float& value) override { insert(name, value); }
    void OnDouble(const char* name, double& value) override { insert(name, value); }

    void OnArrayBegin(const char* name, size_t& /*size*/) override { m_stack.push_back(createArray(name)); }
    void OnArrayEnd() override { m_stack.pop_back(); }

    void OnStructBegin(const char* name) override { m_stack.push_back(createTable(name)); }
    void OnStructEnd() override { m_stack.pop_back(); }

private:
    std::vector<toml::node*> m_stack;

    toml::node* current() { return m_stack.back(); }

    void insert(const char* name, const auto& val)
    {
        toml::node* parent = current();
        if (auto tbl = parent->as_table())
            tbl->insert_or_assign(name, val);
        else if (auto arr = parent->as_array())
            arr->push_back(val);
    }

    toml::node* createTable(const char* name)
    {
        toml::node* parent = current();
        if (auto tbl = parent->as_table())
            return tbl->insert_or_assign(name, toml::table{}).first->second.as_table();
        if (auto arr = parent->as_array())
        {
            arr->push_back(toml::table{});
            return arr->back().as_table();
        }
        return nullptr;
    }

    toml::node* createArray(const char* name)
    {
        toml::node* parent = current();
        if (auto tbl = parent->as_table())
            return tbl->insert_or_assign(name, toml::array{}).first->second.as_array();
        if (auto arr = parent->as_array())
        {
            arr->push_back(toml::array{});
            return arr->back().as_array();
        }
        return nullptr;
    }
};

class CConfigReader : public IConfigValueVisitor
{
public:
    explicit CConfigReader(toml::table& root) { m_stack.push_back({ &root, 0 }); }

    bool IsReading() const override { return true; }
    bool ok() const { return m_ok; }

    void OnInt(const char* name, int& value) override { readScalar(name, value, [](toml::node& n) { return n.as_integer(); }, [](auto* v) { return static_cast<int>(v->get()); }); }
    void OnString(const char* name, std::string& value) override { readScalar(name, value, [](toml::node& n) { return n.as_string(); }, [](auto* v) { return v->get(); }); }
    void OnBool(const char* name, bool& value) override { readScalar(name, value, [](toml::node& n) { return n.as_boolean(); }, [](auto* v) { return v->get(); }); }
    void OnFloat(const char* name, float& value) override { readScalar(name, value, [](toml::node& n) { return n.as_floating_point(); }, [](auto* v) { return static_cast<float>(v->get()); }); }
    void OnDouble(const char* name, double& value) override { readScalar(name, value, [](toml::node& n) { return n.as_floating_point(); }, [](auto* v) { return v->get(); }); }

    void OnArrayBegin(const char* name, size_t& size) override
    {
        if (!m_ok) return;
        if (toml::node* node = fetch(name))
        {
            if (auto arr = node->as_array())
            {
                size = arr->size();
                m_stack.push_back({ arr, 0 });
                return;
            }
        }
        m_ok = false;
    }
    void OnArrayEnd() override { if (m_ok) m_stack.pop_back(); }

    void OnStructBegin(const char* name) override
    {
        if (!m_ok) return;
        if (toml::node* node = fetch(name))
        {
            if (auto tbl = node->as_table())
            {
                m_stack.push_back({ tbl, 0 });
                return;
            }
        }
        m_ok = false;
    }
    void OnStructEnd() override { if (m_ok) m_stack.pop_back(); }

private:
    struct SCtx { toml::node* node; size_t index; };
    std::vector<SCtx> m_stack;
    bool m_ok{ true };

    toml::node* fetch(const char* name)
    {
        SCtx& ctx = m_stack.back();
        if (auto tbl = ctx.node->as_table())
            return tbl->get(name);
        if (auto arr = ctx.node->as_array())
            return ctx.index < arr->size() ? arr->get(ctx.index++) : nullptr;
        return nullptr;
    }

    template<typename T, typename AsFunc, typename GetFunc>
    void readScalar(const char* name, T& out, AsFunc as, GetFunc get)
    {
        if (!m_ok) return;
        if (toml::node* node = fetch(name))
        {
            if (auto val = as(*node))
            {
                out = get(val);
                return;
            }
        }
        m_ok = false;
    }
};

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
    if (!pair.second)
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

    bool configChanged = false;
    for (IConfigValue* mapping : group->m_typeMapping)
    {
        if (!mapping)
        {
            MCLog::error("Config value mapping is null for group {} in namespace {}", group->m_groupName, group->m_namespaceName);
            continue;
        }

        if (!groupTable.get(mapping->GetName()))
        {
            DefaultWriter writer(groupTable);
            mapping->Accept(writer);
            configChanged = true;
            continue;
        }

        CConfigReader reader(groupTable);
        mapping->Accept(reader);
        if (!reader.ok())
            MCLog::error("Config value {} in group {} in namespace {} has invalid type", mapping->GetName(), group->m_groupName, group->m_namespaceName);
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
    if (it != m_registeredConfigs.end())
        return it->second;

    MCLog::error("Config group with type index {} not found", typeIndex.name());
    return nullptr;
}

bool CConfigurationManager::LoadConfigOrGenerateDefault()
{
    const std::ifstream fileStream(m_configPath);
    if (fileStream.good())
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
    if (!result)
    {
        MCLog::error("Failed to parse config file {} with error {}", m_configPath, result.error().description());
        return false;
    }

    m_configTable = result.table();

    for (auto [_, v] : m_registeredConfigs)
    {
        bool hasNamespace = !v->m_namespaceName.empty();
        toml::node* start = nullptr;
        if (hasNamespace)
        {
            if (toml::node* ns = m_configTable.get(v->m_namespaceName))
                if (auto tbl = ns->as_table())
                    start = tbl->get(v->m_groupName);
        }
        else
        {
            start = m_configTable.get(v->m_groupName);
        }

        if (!start || !start->is_table())
            continue;

        toml::table& groupTable = *start->as_table();
        for (IConfigValue* mapping : v->m_typeMapping)
        {
            if (!mapping)
            {
                MCLog::error("Config value mapping is null for group {} in namespace {}", v->m_groupName, v->m_namespaceName);
                continue;
            }
            if (!groupTable.get(mapping->GetName()))
                continue;

            CConfigReader reader(groupTable);
            mapping->Accept(reader);
            if (!reader.ok())
            {
                MCLog::error("Config value {} in group {} in namespace {} has invalid type", mapping->GetName(), v->m_groupName, v->m_namespaceName);
                return false;
            }
        }
    }

    return true;
}
