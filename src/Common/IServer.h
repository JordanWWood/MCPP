#pragma once

struct IServer
{
    virtual ~IServer() = default;
    virtual bool Init() = 0;
    virtual bool Run() = 0;
};