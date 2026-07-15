#pragma once
#include "CommonInclude.h"
#include "Inventory_Info.h"
#include "stbMath.h"
class stbD2DRenderer;

using namespace stb::math;

class DropItemManager
{
public:
   
    void Update();
    void Render(stbD2DRenderer& renderer);

    void AddDropItem(const DropItemData& dropItemData);
    DropItemData* GetDropItem(int dropId);

    void RequestPickup(const Vector2& playerPos);
    int64_t FindPickupItem(const Vector2& playerPos);
   
    void RemoveDropItem(int dropId);
    void Clear();

private:
	std::unordered_map<int, DropItemData> m_dropItems;

};

