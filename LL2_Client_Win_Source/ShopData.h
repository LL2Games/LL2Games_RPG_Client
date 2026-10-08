#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ShopProduct
{
    int productId = 0;
    int itemId = 0;
    std::int64_t price = 0;
    int maxPerPurchase = 0;
};

struct ShopOpenResult
{
    int mapId = 0;
    int spawnId = 0;
    int npcId = 0;
    int shopId = 0;

    std::string name;
    bool allowSell = false;
    std::vector<ShopProduct> products;
};

// 서버가 알려주는 변경 후 슬롯 상태
struct ShopSlotUpdate
{
    int inventoryType = -1;
    int slotPos = -1;
    int itemId = 0;
    int itemCount = 0;
};

struct ShopBuyResult
{
    int mapId = 0;
    int spawnId = 0;
    int shopId = 0;
    int productId = 0;
    int itemId = 0;
    int count = 0;

    std::int64_t totalPrice = 0;
    std::int64_t gold = 0;

    std::vector<ShopSlotUpdate> updatedSlots;
};

struct ShopSellResult
{
    int mapId = 0;
    int spawnId = 0;
    int shopId = 0;
    int itemId = 0;
    int count = 0;

    std::int64_t totalPrice = 0;
    std::int64_t gold = 0;

    ShopSlotUpdate updatedSlot;
};
