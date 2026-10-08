#pragma once

#include <cstdint>

#include "InventoryUI_Info.h"

enum class ShopButtonType
{
    Buy,
    Sell,
    Close
};

struct ShopButton
{
    ShopButtonType type{};
    UIButtonState state = UIButtonState::Normal;

    RECT size{};

    stb::Texture* normalImg = nullptr;
    stb::Texture* hoverImg = nullptr;
    stb::Texture* pressedImg = nullptr;
    stb::Texture* disabledImg = nullptr;

    bool isVisible = true;
    bool isEnable = true;
};

struct ShopProductRow
{
    RECT size{}; // 상점 창 내부 상대 좌표

    int productId = 0;
    int itemId = 0;
    std::int64_t price = 0;

    bool isEnable = false;
    bool isHover = false;
};

struct ShopInventoryRow
{
    RECT size{};

    int inventoryType = -1;
    int slotPos = -1;
    int itemId = 0;
    int itemCount = 0;

    bool isEnable = false;
    bool isHover = false;
};
