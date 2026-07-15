#pragma once
#include "IPacketHandler.h"
#include "Packet.h"
#include "CommonInclude.h" 
#include "Inventory_info.h"

class DropItemPacketHandler : public IPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void HandleSpawnDropItem(const ParsedPacket& pkt);
	static void HandleRemoveDropItem(const ParsedPacket& pkt);
private:
};

