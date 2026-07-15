#pragma once
#include "IPacketHandler.h"
#include "Packet.h"
#include "CommonInclude.h"
#include "..\\LL2_Client_Win_lib\\stbPlayer.h"

class MovePacketHandler : public IPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void SendPlayerMove(stb::Player* player);

private:
};

