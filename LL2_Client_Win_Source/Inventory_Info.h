#pragma once
#include "CommonInclude.h"
#include "stbMath.h"

struct STRInventoryMetaInfo
{
    std::string inventoryType;
    std::string max_slots;
    std::string current_slots_size;
};

struct InventoryMetaInfo
{
    int inventoryType = 0;
    int max_slots = 0;
    int currnet_slots_size = 0;
};

struct InventoryItemInfo
{
    int inventoryType = 0;
    int slotPos = 0;
    int itemId = 0;
    int itemCount = 0;
	int useCount = 0;
};

struct InventorySlot
{
	int inventoryType = 0;
	int slotPos = 0;
	int itemId = 0;
	int itemCount = 0;
	bool isEnable = false;
};

struct InventorySlotUpdate
{
	int slotPos = 0;
	int itemId = 0;
	int itemCount = 0;

};

enum class ItemType
{
	NONE,
	Equip,
	Consume,
	Etc,
	Cash
};

struct ItemData
{
	int itemId = 0;
	std::string name;
	ItemType type = ItemType::Etc;
	bool stackable = false;
	int maxStack = 1;

	int hpRestore = 0;
	int mpRestore = 0;
};

struct DropItemData
{
	DropItemData() = default;
	int dropId = 0;
	int itemId = 0;
	int count = 0;
	stb::math::Vector2 itemPos{};

};

struct PickUpItemData
{
	int inventoryType;
	int slotPos;
	int itemId;
	int itemCount;
};


struct UseItemResult {
	int result;
	int errcode;
	int inventoryType;
	int slotPos;
	int item_id;
	int used_count;
	int remain_count;
	int hp;
	int mp;
};

namespace Item
{
	inline ItemType SetItemType(std::string itemType)
	{
		if (itemType == "Equip") return ItemType::Equip;
		if (itemType == "Consume") return ItemType::Consume;
		if (itemType == "Etc") return ItemType::Etc;
		if (itemType == "Cash") return ItemType::Cash;
		else return ItemType::NONE;
	}
}