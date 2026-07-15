#pragma once
#include "IPacketHandler.h"
#include "Packet.h"
#include "CommonInclude.h"


class ChannelInitPacketHandler : public IPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void SendChannelAuth();
	static void SendEnterMap(const std::string& charId, const std::string& mapId);
	
private:
};




