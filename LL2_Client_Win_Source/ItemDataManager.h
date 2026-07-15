#pragma once
#include "CommonInclude.h"
#include "Inventory_info.h"
#include "stbSingletonBase.h"
#include <nlohmann/json.hpp>

class ItemDataManager : public stb::SingletonBase<ItemDataManager>
{
public:
    bool Init();
    bool PreLoadAll();
    bool LoadJsonFile(const std::string& path, ItemData& itemData);

public:
    const ItemData* FindItemData(int itemId) const;
private:

    std::unordered_map<int, ItemData> m_itemDatas;
};

