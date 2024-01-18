#pragma once

#include "uuid.h"
#include "Packets/IPacket.h"

class SLoginStart : public IPacket
{
public:
    SLoginStart() : IPacket(0x00) {}

    SERIALIZE_BEGIN()
        SERIALIZE_STRING(m_username, 16)
        SERIALIZE_UUID(m_uuid)
    SERIALIZE_END()

    std::string m_username;
    CUUID m_uuid;
};
