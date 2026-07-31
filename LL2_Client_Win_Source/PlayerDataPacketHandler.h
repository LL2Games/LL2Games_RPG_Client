#pragma once

#include "CommonInclude.h"
#include "Packet.h"


class PlayerDataPacketHandler
{
public:
	static void HandleLocalPlayerInfo(const ParsedPacket& pkt);
	static void HandleLocalPlayerStat(const ParsedPacket& pkt);
	static void HandlePlayerOnDamaged(const ParsedPacket& pkt);


	static void HandleLocalPlayerGetExp(const ParsedPacket& pkt);
private:
};

