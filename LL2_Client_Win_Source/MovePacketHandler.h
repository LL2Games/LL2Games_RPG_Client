#pragma once
#include "Packet.h"

class MovePacketHandler
{
public:
    static void Execute(const ParsedPacket& packet);
};
