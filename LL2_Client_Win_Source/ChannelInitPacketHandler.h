#pragma once
#include "Packet.h"
#include "CommonInclude.h"


class ChannelInitPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void SendChannelAuth();
	static void SendEnterMap(const std::string& charId, const std::string& mapId);
	
private:
};




