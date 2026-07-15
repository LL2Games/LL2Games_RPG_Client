#pragma once
#include "CommonInclude.h"
#include "Inventory_Info.h"

class Inventory
{
public:
	Inventory() = default;
	Inventory(InventoryMetaInfo inventoryMetaInfo);
	InventoryItemInfo* FindSlot(int slotPos);
	bool SetSlot(int slotPos, int itemId, int itemCount);
	bool SetSlot(InventoryItemInfo& itemInfo);
	bool RemoveItemFromId(int itemId, int count);
	bool RemoveItem(int slotPos, int count);
	bool RemoveItem(int slotPos);

	std::vector<InventoryItemInfo> GetItemInfos();

	void Clear();

public:
	int GetInventoryType() { return m_inventoryType; }
private:
	int m_inventoryType;
	int m_maxSlot;
	int m_current_slot_size;
	std::unordered_map<int, InventoryItemInfo> m_slots;
};

