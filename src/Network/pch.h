#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <optick.h>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

namespace MCLog = ::spdlog;

