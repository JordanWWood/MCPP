
#include "pch.h"
#include "Core/MCServer.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
    // Set up logging
    spdlog::init_thread_pool(8192, 1);
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("server.log", 1024 * 1024 * 10, 3);
    std::vector<spdlog::sink_ptr> sinks{ stdout_sink, rotating_sink };
    auto logger = std::make_shared<spdlog::async_logger>("MCPP", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    spdlog::set_pattern("[%H:%M:%S %z][%n][%t][%l] %v");

    spdlog::info("Logger initialised");

    // TODO make the port either a console arg or config option
    CMCServer server(25565);

    if (server.Init())
    {
        while(server.Run()) {}
    }
    
    return 0;
}
