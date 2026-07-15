#pragma once
#include "stbTexture.h"
enum class InventoryType
{
    Equip,
    Consume,
    Etc,
    Setup,
    Cash,
    Cosmetic,
    EnumEnd
};

enum class InventoryButtonType
{
    Expand,
    Reduce,
    Close,
    EnumEnd
};

enum class UIButtonState
{
    Normal,
    Hover,
    Pressed
};

struct ExpBarTextureKey
{
    std::wstring layerBack;
    std::wstring gauge;
};

struct InventorySlotUI
{
    int slotIndex = -1;

    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    bool isEnabled = false;

    int itemId = 0;
    int itemCount = 0;
};


struct InventoryButton
{
    InventoryButtonType type;
    UIButtonState state;

    RECT size;

    stb::Texture* nomalImg;
    stb::Texture* hoverImg;
    stb::Texture* pressedImg;

    bool isVisble;
    bool isEnable;
};

struct InventoryTabVisual
{
    stb::Texture* normal = nullptr;
    stb::Texture* selected = nullptr;
    RECT rect{};
};

struct InventoryTabButton
{
    InventoryType type{};
    InventoryTabVisual normalView;
    InventoryTabVisual fullView;
};

/*

enum class InventoryType
{
    Equip,
    Consume,
    Etc,
    Setup,
    Cash,
    Cosmetic,
    EnumEnd
};
*/
namespace inventoryConvert
{
    inline InventoryType SetInvenType(int num)
    {
        switch (num)
        {
        case 0 : return InventoryType::Equip;
        case 1 : return InventoryType::Consume;
        case 2 : return InventoryType::Etc;
        case 3 : return InventoryType::Setup;
        case 4 : return InventoryType::Cash;
        case 5 : return InventoryType::Cosmetic;
        default: return InventoryType::EnumEnd;
        }
    }
}