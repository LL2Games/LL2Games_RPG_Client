#pragma once
#include "IPacketHandler.h"
#include "Packet.h"
#include "CommonInclude.h"
#include "Inventory_Info.h"
#include "QuickSlotManager.h"

class QuickSlotPacketHandler : public IPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void HandleSlotList(const ParsedPacket& pkt);
	static void HandleSlotSet(const ParsedPacket& pkt);
	static void SendSetQuickSlot(const QuickSlotData& quickSlotData);
private:
};

