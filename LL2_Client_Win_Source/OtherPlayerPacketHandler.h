#pragma once
#include "IPacketHandler.h"
#include "Packet.h"
#include "CommonInclude.h"

class OtherPlayerPacketHandler : public IPacketHandler
{
public:
	static void HandleOtherPlayerEnter(const ParsedPacket& pkt);
	static void HandleOtherPlayerSnapShot(const ParsedPacket& pkt);
	static void Execute(const ParsedPacket& pkt);
	
private:
};

