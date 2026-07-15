#include "InventoryManager.h"


bool InventoryManager::CreateInventory(InventoryMetaInfo inventoryMetaInfo)
{
	auto it = m_inventories.find(inventoryMetaInfo.inventoryType);
	if (it != m_inventories.end())
	{
		return false;
	}
	m_inventories.emplace(inventoryMetaInfo.inventoryType, Inventory(inventoryMetaInfo));
	return true;
}

void InventoryManager::EnsureInventory(InventoryMetaInfo inventoryMetaInfo)
{
	auto it = m_inventories.find(inventoryMetaInfo.inventoryType);
	if (it == m_inventories.end())
	{
		m_inventories.emplace(inventoryMetaInfo.inventoryType, Inventory(inventoryMetaInfo));
	}
}


Inventory* InventoryManager::GetInventory(int inventoryType)
{
	auto it = m_inventories.find(inventoryType);

	if (it == m_inventories.end())
	{
		OutputDebugStringA("GetInventory fail\n");
		return nullptr;
	}
	return &(it->second);
}

const Inventory* InventoryManager::GetInventory(int inventoryType) const
{
	auto it = m_inventories.find(inventoryType);
	if (it == m_inventories.end())
	{
		return nullptr;
	}

	return &(it->second);
}

InventoryItemInfo* InventoryManager::FindSlot(int inventoryType, int slotPos)
{
	auto it = m_inventories.find(inventoryType);
	
	if (it == m_inventories.end())
	{
		return nullptr;
	}
	return it->second.FindSlot(slotPos);
}

void InventoryManager::Clear()
{
	m_inventories.clear();
}

void InventoryManager::ClearItems()
{
	for (auto& [inventoryType, inventory] : m_inventories)
	{
		inventory.Clear();
	}
}