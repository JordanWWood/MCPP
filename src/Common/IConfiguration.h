#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct SServer
{
    std::string m_name;
    std::string m_address;
    uint16_t m_port{ 25565 };
    bool isDefault{ false };
};

struct IConfiguration
{
    virtual ~IConfiguration() = default;

    virtual bool IsOnline() const = 0;
    virtual uint16_t GetHostPort() const = 0;
    virtual const std::vector<SServer>& GetPredefinedServers() const = 0;
};
