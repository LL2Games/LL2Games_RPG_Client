#pragma once
#include "stbSingletonBase.h"
#include "CommonInclude.h"
#include "ItemDataManager.h"
#include "ItemPacketHandler.h"
#include "..//LL2_Client_Win_lib/stbPlayer.h"

// 플레이어가 아이템

class ItemUseService : public stb::SingletonBase<ItemUseService>
{

public:
	bool UseItem(stb::Player* player, int inventoryType, int slotPos);


private:


};

