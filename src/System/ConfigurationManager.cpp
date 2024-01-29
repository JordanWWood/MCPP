#include "pch.h"
#include "ConfigurationManager.h"

#define TOML_IMPLEMENTATION
#include <toml++/toml.hpp>

using namespace std::string_view_literals;
static constexpr auto g_defaultConfig = R"([general]
# Disabling this will allow non genuine clients to connect to the proxy
online_mode = true
# The port MCPP will run on by default
host_port = 25565

[[servers]]
name = "Default"
address = "127.0.0.1"
port = 25566

# Clients will connect to this server by default. 
# If no default is defined a server will be selected round robin
default = true
)"sv;

CConfigurationManager::CConfigurationManager(std::string&& configPath)
    : m_configPath(std::move(configPath))
{
}

bool CConfigurationManager::Init()
{
    return LoadConfigOrGenerateDefault();
}

bool CConfigurationManager::LoadConfigOrGenerateDefault()
{
    const std::ifstream fileStream(m_configPath);
    if(fileStream.good())
        return LoadConfig();

    MCLog::info("Could not find config. Generating default config {}", m_configPath);
    
    std::ofstream out(m_configPath);
    out << g_defaultConfig;
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

    toml::table tbl = result.table();
    toml::node_view serversView = tbl["servers"];
    toml::array* arr = tbl["servers"].as_array();
    if(!arr)
    {
        MCLog::error(
            "No servers defined in config {}. MCPP needs at least one registered server to operate correctly. "
            "Functionality to allow no servers to be defined at startup is coming soon...", m_configPath);

        return false;
    }
    
    int size = arr->size();
    bool successfulParse{ true };
    for(int i = 0; i < size; ++i)
    {
        toml::node_view serverTable = serversView[i];
        if(!serverTable.is_table())
        {
            MCLog::warn("Unknown element in Servers. Element will be ignored");
            continue;
        }
        
        SServer newServer;

        std::string& serverName = newServer.m_name;
        if(toml::value<std::string>* name = serverTable["name"].as_string())
            serverName = name->get();
                
        if(toml::value<std::string>* address = serverTable["address"].as_string())
            newServer.m_address = address->get();
        else
        {
            MCLog::error("Server {} cannot parse address", serverName);
            successfulParse = false;
            break;
        }

        if(toml::value<int64_t>* port = serverTable["port"].as_integer())
            newServer.m_port = port->get();
        else
            MCLog::warn("Server {} cannot parse port. Port will default to 25565", serverName);

        if(toml::value<bool>* isDefault = serverTable["default"].as_boolean())
            newServer.isDefault = isDefault->get();
            
        m_servers.emplace_back(std::move(newServer));
    }

    const toml::node_view general = tbl["general"];
    if(!general)
    {
        MCLog::warn("\"general\" is not defined in {}. Will fall back on defaults", m_configPath);
        return true;
    }

    if(toml::value<bool>* onlineMode = general["online_mode"].as_boolean())
        m_isOnline = onlineMode->get();
    else
        MCLog::warn("\"general:online_mode\" is not defined. Defaulting to {}", m_isOnline);

    if(toml::value<int64_t>* hostPort = general["host_port"].as_integer())
        m_hostPort = hostPort->get();
    else
        MCLog::warn("\"general:host_port\" is not defined. Defaulting to {}", m_hostPort);
    
    return successfulParse;
}
