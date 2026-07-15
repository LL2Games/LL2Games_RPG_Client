#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "Inventory.h"
#include "Inventory_Info.h"

class InventoryManager : public stb::SingletonBase<InventoryManager>
{
public:

	bool CreateInventory(InventoryMetaInfo inventoryMetaInfo);
	void EnsureInventory(InventoryMetaInfo inventoryMetaInfo);
	void UpdateSlot(InventorySlotUpdate& inventorySlotUpdate);

	void ClearItems();
public:
	
	Inventory* GetInventory(int inventoryType);
	const Inventory* GetInventory(int inventoryType) const;

	InventoryItemInfo* FindSlot(int inventoryType, int slotPos);
	//const InventorySlot* FindSlot(int inventoryType, int slotPos) const;

	void Clear();
private:

	std::unordered_map<int, Inventory> m_inventories;
	
	
};

