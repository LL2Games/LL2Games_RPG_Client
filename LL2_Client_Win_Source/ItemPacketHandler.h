#pragma once
#include "IPacketHandler.h"
#include "Packet.h"
#include "CommonInclude.h"
#include "Inventory_Info.h"


class ItemPacketHandler : public IPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void HandleUseItemResult(const ParsedPacket& pkt);
	static void HandlePickUpItem(const ParsedPacket& pkt);
	static void SendUseItem(InventoryItemInfo* inventoryitemInfo);
	static void SendPickupDropItem(int64_t dropId);
	
private:
};


