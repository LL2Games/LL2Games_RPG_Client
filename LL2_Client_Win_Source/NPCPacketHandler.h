#pragma once

#include "Packet.h"

class NPCPacketHandler
{
public:
    static void HandleNPCSnapshot(const ParsedPacket& packet);
};
