#pragma once
#include "Packet.h"
#include "CommonInclude.h"
#include "..\\LL2_Client_Win_lib\\stbPlayer.h"

class MovePacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void SendPlayerMove(stb::Player* player);

private:
};

