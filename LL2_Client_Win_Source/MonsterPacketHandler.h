#pragma once
#include "Packet.h"
#include "CommonInclude.h"

class MonsterPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void HandleS2C_SpawnMonster(const ParsedPacket& pkt);
	static void HandleS2C_MonsterMove(const ParsedPacket& pkt);
	static void HandleS2C_RespawnMonster(const ParsedPacket& pkt);

private:
};

