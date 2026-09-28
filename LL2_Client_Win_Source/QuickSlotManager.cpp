#include "QuickSlotManager.h"
#include "ItemPacketHandler.h"
#include "InventoryManager.h"
#include "CombatPacketHandler.h"
#include "PlayerManager.h"
#include "stbInput.h"
#include "QuickSlotPacketHandler.h"
#include "CombatSystem.h"
#include "..\\LL2_Client_Win_lib\\\stbPlayer.h"

#define M_INVENTORYMANAGER stb::SingletonBase<InventoryManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()

void QuickSlotManager::Init()
{
    std::string msg =
        "QuickSlotManager::Init this: " +
        std::to_string(reinterpret_cast<uintptr_t>(this)) +
        ", m_maxSlotCount: " +
        std::to_string(m_maxSlotCount) +
        "\n";

    OutputDebugStringA(msg.c_str());

    m_slots.clear();
    m_slots.resize(m_maxSlotCount);

    for (int i = 0; i < m_maxSlotCount; ++i)
    {
        m_slots[i].slot_index = i;
    }
}

void QuickSlotManager::Update()
{
 
}

void QuickSlotManager::SetSlot(QuickSlotData& quickSlotData)
{
    if (quickSlotData.slot_index < 0 || quickSlotData.slot_index >= (int)m_slots.size())
        return;

    m_slots[quickSlotData.slot_index] = quickSlotData;
}

void QuickSlotManager::SetSlotSkill(int /*slotIndex*/, int /*skillId*/)
{

}

void QuickSlotManager::SetSlotItem(int /*slotIndex*/, int /*inventoryType*/, int /*slotPos*/)
{

}

// 퀵슬롯 변경 하는 요청 서버로 보냄
void QuickSlotManager::RequestSetSlot(const QuickSlotData& quickSlotData)
{
    if (quickSlotData.slot_index < 0 || quickSlotData.slot_index >= m_maxSlotCount)
        return;
    QuickSlotPacketHandler::SendSetQuickSlot(quickSlotData);

}

void QuickSlotManager::ClearSlot(int /*slotIndex*/)
{

}

const QuickSlotData* QuickSlotManager::GetSlot(int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= (int)m_slots.size())
        return nullptr;

    return &m_slots[slotIndex];
}

void QuickSlotManager::UseSlot(int slotIndex)
{
   
    if (slotIndex < 0 || slotIndex >= static_cast<int>(m_slots.size()))
    {
        OutputDebugStringA("QuickSlotManager::UseSlot - slotIndex out of range\n");
        return;
    }

    QuickSlotData& slot = m_slots[slotIndex];

    switch (slot.type)
    {
    case QuickSlotType::Skill:
    {
        stb::Player* player = M_PLAYERMANAGER->GetLocalPlayer();
        if (player == nullptr)
            return;

        CombatSystem* combatSystem = player->GetCombatSystem();
        if (combatSystem == nullptr)
            return;

        combatSystem->TrySkillAttack(slot.ref_id);
        break;
    }
    case QuickSlotType::Item:
    {
        Inventory* inven = M_INVENTORYMANAGER->GetInventory(static_cast<int>(slot.inventory_type));

        if (inven == nullptr)
            return;


        InventoryItemInfo* itemInfo = inven->FindSlot(slot.inventory_slotPos);
        if (itemInfo == nullptr)
            return;

        if (itemInfo->itemId != slot.ref_id)
            return;
        itemInfo->useCount = 1;
        ItemPacketHandler::SendUseItem(itemInfo);
        break;
    }
    default:
        break;
    }
}



