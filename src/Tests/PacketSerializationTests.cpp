#include <doctest.h>
#include "PacketUtils.h"
#include "Packets/PacketReader.h"
#include "Packets/Handshake/Handshake.h"
#include "Packets/Login/LoginStart.h"
#include "Uuid.h"

TEST_CASE("Handshake packet serialization roundtrip") {
    SHandshake handshake;
    handshake.m_protocolVersion = 760;
    handshake.m_address = "example.com";
    handshake.m_port = 25565;
    handshake.m_nextState = static_cast<uint8_t>(EClientState::eCS_Login);

    SPacketPayload payload = SerializePacket(handshake);
    CHECK(payload.m_packetId == handshake.m_packetId);

    SHandshake parsed;
    CPacketReader reader(payload.m_payload);
    parsed.Serialize(reader);

    CHECK(parsed.m_packetId == handshake.m_packetId);
    CHECK(parsed.m_packetSize == handshake.m_packetSize);
    CHECK(parsed.m_protocolVersion == handshake.m_protocolVersion);
    CHECK(parsed.m_address == handshake.m_address);
    CHECK(parsed.m_port == handshake.m_port);
    CHECK(parsed.m_nextState == handshake.m_nextState);
}

TEST_CASE("LoginStart packet serialization roundtrip") {
    SLoginStart login;
    login.m_username = "TestUser";
    login.m_uuid = CUUID::fromStrFactory("123e4567-e89b-12d3-a456-426614174000");

    SPacketPayload payload = SerializePacket(login);
    CHECK(payload.m_packetId == login.m_packetId);

    SLoginStart parsed;
    CPacketReader reader(payload.m_payload);
    parsed.Serialize(reader);

    CHECK(parsed.m_packetId == login.m_packetId);
    CHECK(parsed.m_packetSize == login.m_packetSize);
    CHECK(parsed.m_username == login.m_username);
    CHECK(parsed.m_uuid == login.m_uuid);
}