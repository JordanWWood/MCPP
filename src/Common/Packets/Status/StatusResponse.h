#pragma once
#include "Packets/IPacket.h"
#include "PacketPayload.h"

#include "nlohmann/json.hpp"

struct SStatusResponse : public IPacket
{
    SStatusResponse() : IPacket(0x00) {}

    SERIALIZE_BEGIN()
        std::string body = nlohmann::to_string(m_body);
        SERIALIZE_STRING(body, MAX_STRING_LENGTH)
    SERIALIZE_END()

    nlohmann::json m_body;
};
