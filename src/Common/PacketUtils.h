#pragma once

#include "PacketPayload.h"
#include "Packets/IPacket.h"
#include "Packets/PacketSizeCalc.h"
#include "Packets/PacketWriter.h"

static SPacketPayload SerializePacket(IPacket& packet)
{
    CPacketSizeCalc sizeCalc;
    packet.Serialize(sizeCalc);

    packet.m_packetSize = sizeCalc.GetPayloadSize();

    CPacketWriter writer(sizeCalc.GetFullSize());
    packet.Serialize(writer);

    SPacketPayload payload;
    payload.m_size = sizeCalc.GetFullSize();
    payload.m_packetId = packet.m_packetId;
    payload.m_payload = writer.GetData();
    
    return payload;
}
