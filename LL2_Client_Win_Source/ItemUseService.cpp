#include "ItemUseService.h"



bool ItemUseService::UseItem(stb::Player* player, int inventoryType, int slotPos)
{
    if (player == nullptr)
        return false;

    InventoryManager* inventoryManager = player->GetInvenManager();
    if (inventoryManager == nullptr)
        return false;

    Inventory* inventory = inventoryManager->GetInventory(inventoryType);
    if (inventory == nullptr)
        return false;

    InventoryItemInfo* itemInfo = inventory->FindSlot(slotPos);
    if (itemInfo == nullptr)
        return false;

    if (itemInfo->itemId == 0 || itemInfo->itemCount <= 0)
        return false;

    const ItemData* itemData = ItemDataManager::getInstance()->FindItemData(itemInfo->itemId);
    if (itemData == nullptr)
        return false;

    if (itemData->type != ItemType::Consume)
        return false;


    ItemPacketHandler::SendUseItem(itemInfo);
    return true;
}
