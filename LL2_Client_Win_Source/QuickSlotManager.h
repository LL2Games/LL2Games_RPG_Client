#pragma once
#include "CommonInclude.h"
#include "stbInput.h"
#include "InventoryUI_Info.h"
#include "QuickSlotUI_Info.h"

class QuickSlotManager
{
public:
    void Init();
    void Update();

    void ClearSlot(int slotIndex);
    void UseSlot(int slotIndex);
public:
    //void SetOwner(stb::Player* player) { m_owner = player; }
    void SetSlot(QuickSlotData& quickSlotData);
    void SetSlotSkill(int slotIndex, int skillId);
    void SetSlotItem(int slotIndex, int inventoryType, int slotPos);

    void RequestSetSlot(const QuickSlotData& quickSlotData);

    const QuickSlotData* GetSlot(int slotIndex) const;
   

public:
   


private:
    std::vector<QuickSlotData> m_slots;
    //stb::Player* m_owner;

private:
    static constexpr int m_maxSlotCount = 32;

};


namespace QuickSlot
{
    inline QuickSlotType SetQuickSlotType(int type)
    {
        switch (type)
        {
        case 1: return QuickSlotType::Skill;
        case 2: return QuickSlotType::Item;
        default: return QuickSlotType::None;
        }
    }
}
