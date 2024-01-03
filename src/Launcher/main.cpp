
#include "pch.h"
#include "MCServer.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

#include "TCPServer.h"

int main(int argc, char** argv)
{
    OPTICK_THREAD("Main Thread");

    // Set up logging
    MCLog::init_thread_pool(8192, 1);
    auto stdout_sink = std::make_shared<MCLog::sinks::stdout_color_sink_mt>();
    auto rotating_sink = std::make_shared<MCLog::sinks::rotating_file_sink_mt>("server.log", 1024 * 1024 * 10, 3);
    std::vector<MCLog::sink_ptr> sinks{ stdout_sink, rotating_sink };
    auto logger = std::make_shared<MCLog::async_logger>("MCPP", sinks.begin(), sinks.end(), MCLog::thread_pool(), MCLog::async_overflow_policy::block);
    MCLog::register_logger(logger);
    MCLog::set_default_logger(logger);

#ifdef _DEBUG
    MCLog::set_level(MCLog::level::debug);
#endif

    MCLog::set_pattern("[%H:%M:%S %z][%n][%t][%l] %v");

    MCLog::info("Logger initialised");

    std::unique_ptr<CTCPServer> tcpServer = std::make_unique<CTCPServer>(25565);
    // TODO make the port either a console arg or config option
    CMCServer server(std::move(tcpServer));

    if (server.Init())
    {
        while(server.Run()) {}
    }
    else
    {
        MCLog::critical("Failed to initialise MCServer... Exiting");
    }
    
    return 0;
}
