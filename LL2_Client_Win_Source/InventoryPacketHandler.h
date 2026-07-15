#pragma once

#include "IPacketHandler.h"
#include "Packet.h"
#include "CommonInclude.h" 
#include "Inventory_info.h"

class InventoryPacketHandler : public IPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void HandleInventoryMetaInfo(const ParsedPacket& pkt);
	static void HandleInventoryItemInfo(const ParsedPacket& pkt);
	static void HandleInventoryMoveItem(const ParsedPacket& pkt);
	static void SendMoveItem(int inventoryType, int fromSlotPos, int toSlotPos);
	static bool ParseNextIntField(const char* data, size_t payloadSize, size_t& offset, int& outValue, std::string& errMsg);
private:

};

