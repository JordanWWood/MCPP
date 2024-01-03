#pragma once

enum class ESocketState
{
    eSS_UNINITIALISED,
    eSS_INVALID_ADDR,
    eSS_INVALID,
    eSS_BIND_ERROR,
    eSS_LISTEN_ERROR,
    eSS_LISTEN,
    eSS_CONNECTED,
    eSS_CLOSED
};
