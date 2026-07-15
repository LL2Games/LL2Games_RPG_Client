#pragma once
#include "InventoryUI_Info.h"

enum class QuickSlotType
{
    None = 0,
    Skill,
    Item,
    UI,
    Emote
};


struct QuickSlotData
{
    QuickSlotType type = QuickSlotType::None;
    int slot_index = 0;
    int ref_id = 0;
    InventoryType inventory_type = InventoryType::EnumEnd;
    int inventory_slotPos = 0;
    int count = 0;
};
