#include "ShopUI.h"

#include "ShopManager.h"
#include "ShopPacketHandler.h"
#include "PlayerManager.h"
#include "stbResourceManager.h"
#include "stbD2DRenderer.h"
#include "stbApplication.h"
#include "stbInput.h"
#include "ItemDataManager.h"
#include "StringConvert.h"


#include <algorithm>
#include <cstdlib>

#define M_REMANAGER stb::ResourceManager::getInstance()
#define M_INPUT stb::Input::getInstance()
#define M_APP stb::Application::getInstance()

void ShopUI::Init()
{
    m_background = M_REMANAGER->Find<stb::Texture>(L"Shop_background");

    Init_ShopButton();

    m_productRowNormal = M_REMANAGER->Find<stb::Texture>(L"Shop_shop_item_row_normal");
    m_productRowHover = M_REMANAGER->Find<stb::Texture>(L"Shop_shop_item_row_mouseOver");
    m_productRowSelected = M_REMANAGER->Find<stb::Texture>(L"Shop_shop_item_row_selected");
    m_inventoryRowNormal = M_REMANAGER->Find<stb::Texture>(L"Shop_inventory_item_row_normal");
    m_inventoryRowHover = M_REMANAGER->Find<stb::Texture>(L"Shop_inventory_item_row_mouseOver");
    m_inventoryRowSelected = M_REMANAGER->Find<stb::Texture>(L"Shop_inventory_item_row_selected");
    m_coinImg = M_REMANAGER->Find<stb::Texture>(L"38071931");

    CreateProductRows();
    CreateInventoryRows();
    UpdateProductRows();
    ResetSelection();

    // 상점 표시 여부는 ShopManager가 관리한다.
    mActive = true;
}

void ShopUI::Init_ShopButton()
{ 
    m_closeButton = {
        ShopButtonType::Close,
        UIButtonState::Normal,
        kCloseRect,
        M_REMANAGER->Find<stb::Texture>(L"Shop_button_close_normal"),
        M_REMANAGER->Find<stb::Texture>(L"Shop_button_close_mouseOver"),
        M_REMANAGER->Find<stb::Texture>(L"Shop_button_close_pressed"),
        M_REMANAGER->Find<stb::Texture>(L"Shop_button_close_disabled"),
        true,
        true
    };

}

void ShopUI::ResetSelection()
{
    m_page = 0;
    m_selectedProductId = 0;
    m_sellPage = 0;
    m_selectedSellInventoryType = -1;
    m_selectedSellSlotPos = -1;
    m_selectedSellItemId = 0;

    m_isShopDragging = false;
    m_lastClickValid = false;
    m_closeButton.state = UIButtonState::Normal;

    for (auto& row : m_inventoryRows)
    {
        row.isEnable = false;
        row.isHover = false;
    }
}

void ShopUI::Update()
{
    const auto* shop = ShopManager::getInstance()->GetCurrentShop();

    if (shop == nullptr)
        return;

    auto* player = PlayerManager::getInstance()->GetLocalPlayer();

    if (player == nullptr || player->IsDead() || player->GetPlayerLocation()->mapId != shop->mapId)
    {
        ShopManager::getInstance()->Close();
        ResetSelection();
        return;
    }

    POINT point{};

    if (!GetCursorPos(&point) || !ScreenToClient(M_APP->GetHWND(), &point))
    {
        return;
    }

    const int productCount = static_cast<int>(shop->products.size());

    if (!m_isShopDragging)
    {
        if (M_INPUT->GetKeyDown(stb::eKeyCode::PageUp) && m_page > 0)
        {
            --m_page;
            m_selectedProductId = 0;
            m_lastClickValid = false;
        }

        if (M_INPUT->GetKeyDown(stb::eKeyCode::PageDown) && (m_page + 1) * kPageSize < productCount)
        {
            ++m_page;
            m_selectedProductId = 0;
            m_lastClickValid = false;
        }
    }

    UpdateProductRows();
    UpdateInventoryRows();


    if (M_INPUT->GetKey(stb::eKeyCode::LButton))
        HandleDragging(point.x, point.y);

    UpdateButtonState(point.x, point.y);
    UpdateProductRowState(point.x, point.y);
    UpdateInventoryRowState(point.x, point.y);

    if (M_INPUT->GetKeyDown(stb::eKeyCode::RButton))
    {
        HandleRMouseClick(point.x, point.y);
    }
    else if (M_INPUT->GetKeyDown(stb::eKeyCode::LButton))
    {
        HandleLMouseClick(point.x, point.y);
    }

    if (M_INPUT->GetKeyUp(stb::eKeyCode::LButton))
        HandleMouseUp();
}

void ShopUI::Render(stbD2DRenderer& renderer)
{
    if (!ShopManager::getInstance()->IsOpen())
        return;

    RenderBackground(renderer);
    RenderProductRows(renderer);
    RenderInventoryRows(renderer);
    RenderButtons(renderer);
    RenderGold(renderer);
}

void ShopUI::RenderBackground(stbD2DRenderer& renderer)
{
    if (m_background == nullptr)
        return;

    auto* bitmap = m_background->GetD2DBitmap();

    if (bitmap == nullptr)
        return;

    const auto size = bitmap->GetSize();

    renderer.DrawBitmap(
        bitmap,
        m_ShopImgPosX,
        m_ShopImgPosY,
        size.width,
        size.height,
        1.0f);
}

void ShopUI::RenderButtons(stbD2DRenderer& renderer)
{
    const auto drawButton = [&](const ShopButton& button)
        {
            if (!button.isVisible)
                return;

            auto* texture = GetCurrentImg(button);

            if (texture == nullptr)
                return;

            auto* bitmap = texture->GetD2DBitmap();

            if (bitmap == nullptr)
                return;

            const auto size = bitmap->GetSize();

            renderer.DrawBitmap(
                bitmap,
                m_ShopImgPosX + button.size.left,
                m_ShopImgPosY + button.size.top,
                size.width,
                size.height,
                1.0f);
        };

    drawButton(m_closeButton);
}

void ShopUI::RenderProductRows(stbD2DRenderer& renderer)
{
    const D2D1::ColorF textColor(0.95f, 0.95f, 0.88f);

    for (const auto& row : m_productRows)
    {
        if (!row.isEnable)
            continue;

        const float x = m_ShopImgPosX + row.size.left;
        const float y = m_ShopImgPosY + row.size.top;

        stb::Texture* rowTexture = m_productRowNormal;

        if (row.productId == m_selectedProductId)
            rowTexture = m_productRowSelected;
        else if (row.isHover)
            rowTexture = m_productRowHover;

        if (rowTexture != nullptr)
        {
            auto* bitmap = rowTexture->GetD2DBitmap();

            if (bitmap != nullptr)
                renderer.DrawBitmap(bitmap, x, y, 336, 44, 1.0f);
        }

        // 인벤토리와 동일하게 아이템 ID로 이미지 조회
        auto* icon = M_REMANAGER->Find<stb::Texture>( std::to_wstring(row.itemId));

        if (icon != nullptr)
        {
            auto* bitmap = icon->GetD2DBitmap();

            if (bitmap != nullptr)
                renderer.DrawBitmap(bitmap, x + 5, y + 6, 32, 32, 1.0f);
        }

        const auto* item = ItemDataManager::getInstance()->FindItemData(row.itemId);

        const std::wstring name = item != nullptr ? Convert::Utf8ToWstr(item->name): L"아이템 " + std::to_wstring(row.itemId);

        renderer.DrawTextString(
            name,
            D2D1::RectF(x + 48, y + 2, x + 328, y + 22),
            textColor,
            TextStyle::Small);


        if (m_coinImg != nullptr)
        {
            auto* bitmap = m_coinImg->GetD2DBitmap();

            if (bitmap != nullptr)
                renderer.DrawBitmap(bitmap, x + 48, y + 24, 14, 14, 1.0f);
        }

        renderer.DrawTextString(
            std::to_wstring(row.price),
            D2D1::RectF(x + 68, y + 23, x + 328, y + 43),
            textColor,
            TextStyle::Small);
    }
}

void ShopUI::RenderInventoryRows(stbD2DRenderer& renderer)
{
    const D2D1::ColorF textColor(0.95f, 0.95f, 0.88f);

    for (const auto& row : m_inventoryRows)
    {
        if (!row.isEnable)
            continue;

        const float x = m_ShopImgPosX + row.size.left;
        const float y = m_ShopImgPosY + row.size.top;

        const bool selected =
            row.inventoryType == m_selectedSellInventoryType &&
            row.slotPos == m_selectedSellSlotPos &&
            row.itemId == m_selectedSellItemId;

        auto* texture = selected
            ? m_inventoryRowSelected
            : row.isHover
            ? m_inventoryRowHover
            : m_inventoryRowNormal;

        if (texture != nullptr)
        {
            auto* bitmap = texture->GetD2DBitmap();

            if (bitmap != nullptr)
                renderer.DrawBitmap(bitmap, x, y, 278, 44, 1.0f);
        }

        auto* icon = M_REMANAGER->Find<stb::Texture>(std::to_wstring(row.itemId));

        if (icon != nullptr)
        {
            auto* bitmap = icon->GetD2DBitmap();

            if (bitmap != nullptr)
                renderer.DrawBitmap(bitmap, x + 5, y + 6, 32, 32, 1.0f);
        }

        const auto* item = ItemDataManager::getInstance()->FindItemData(row.itemId);

        const std::wstring name = item != nullptr ? Convert::Utf8ToWstr(item->name) : L"아이템 " + std::to_wstring(row.itemId);

        renderer.DrawTextString(
            name,
            D2D1::RectF(x + 48, y + 2, x + 270, y + 22),
            textColor,
            TextStyle::Small);

        const bool showCount = row.inventoryType == static_cast<int>(InventoryType::Consume) || row.inventoryType == static_cast<int>(InventoryType::Etc);

        if (showCount)
        {
            renderer.DrawTextString(
                std::to_wstring(row.itemCount),
                D2D1::RectF(x + 5, y + 25, x + 37, y + 38),
                textColor,
                TextStyle::Small);
        }

        if (m_coinImg != nullptr)
        {
            auto* bitmap = m_coinImg->GetD2DBitmap();

            if (bitmap != nullptr)
                renderer.DrawBitmap(bitmap, x + 48, y + 24, 14, 14, 1.0f);
        }

        renderer.DrawTextString(
            item != nullptr ? std::to_wstring(item->sellPrice) : L"?",
            D2D1::RectF(x + 68, y + 23, x + 270, y + 43),
            textColor,
            TextStyle::Small);
    }
}

void ShopUI::RenderGold(stbD2DRenderer& renderer)
{
    auto* player = PlayerManager::getInstance()->GetLocalPlayer();

    if (player == nullptr)
        return;

    const auto gold = player->GetGold();

    const float x = m_ShopImgPosX;
    const float y = m_ShopImgPosY;

    const D2D1::ColorF color(0.95f, 0.95f, 0.88f);

    if (m_coinImg != nullptr)
    {
        auto* bitmap = m_coinImg->GetD2DBitmap();

        if (bitmap != nullptr)
        {
            renderer.DrawBitmap(
                bitmap,
                x + 492,
                y + 40,
                14,
                14,
                1.0f);
        }
    }

    renderer.DrawTextString(
        gold.has_value() ? std::to_wstring(*gold) : L"확인 중",
        D2D1::RectF(x + 512, y + 35, x + 680, y + 60),
        color,
        TextStyle::Money);
}

void ShopUI::UpdateButtonState(int mouseX, int mouseY)
{
    const int localX = static_cast<int>(mouseX - m_ShopImgPosX);
    const int localY = static_cast<int>(mouseY - m_ShopImgPosY);

    UpdateSingleButtonState(m_closeButton, localX, localY);
}

void ShopUI::UpdateSingleButtonState( ShopButton& button, int localX, int localY)
{
    if (!button.isVisible || !button.isEnable || m_isShopDragging || !IsPointInRect(button.size, localX, localY))
    {
        button.state = UIButtonState::Normal;
        return;
    }

    const bool pressed = M_INPUT->GetKey(stb::eKeyCode::LButton) ||  M_INPUT->GetKeyDown(stb::eKeyCode::LButton);

    button.state = pressed ? UIButtonState::Pressed : UIButtonState::Hover;
}

void ShopUI::HandleLMouseClick(int mouseX, int mouseY)
{
    if (HandleButtonClick(mouseX, mouseY))
    {
        m_lastClickValid = false;
        return;
    }
       

    if (HandleProductClick(mouseX, mouseY))
    {
        if (CheckDoubleClick(false, m_selectedProductId, -1, -1, mouseX, mouseY))
        {
            ShopPacketHandler::SendBuy(m_selectedProductId, 1);
        }

        return;
    }
        

    if (HandleInventoryRowClick(mouseX, mouseY))
    {
        if (CheckDoubleClick(true, m_selectedSellItemId, m_selectedSellInventoryType, m_selectedSellSlotPos, mouseX, mouseY))
        {
            ShopPacketHandler::SendSell(m_selectedSellInventoryType, m_selectedSellSlotPos, 1);
        }

        return;
    }

    m_lastClickValid = false;
        
    const int localX = static_cast<int>(mouseX - m_ShopImgPosX);
    const int localY = static_cast<int>(mouseY - m_ShopImgPosY);

    if (IsPointInRect(kTitleRect, localX, localY))
    {
        m_isShopDragging = true;
        m_dragOffsetX = mouseX - m_ShopImgPosX;
        m_dragOffsetY = mouseY - m_ShopImgPosY;
    }
}

bool ShopUI::HandleButtonClick(int mouseX, int mouseY)
{
    const int localX = static_cast<int>(mouseX - m_ShopImgPosX);
    const int localY = static_cast<int>(mouseY - m_ShopImgPosY);

    if (m_closeButton.isVisible && m_closeButton.isEnable && IsPointInRect(m_closeButton.size, localX, localY))
    {
        ShopManager::getInstance()->Close();
        ResetSelection();
        return true;
    }
    return false;
}

bool ShopUI::CheckDoubleClick(bool inventory, int itemKey, int inventoryType, int slotPos, int mouseX, int mouseY)
{
    const ULONGLONG now = GetTickCount64();

    const bool doubleClick =
        m_lastClickValid &&
        m_lastClickInventory == inventory &&
        m_lastClickKey == itemKey &&
        m_lastClickInventoryType == inventoryType &&
        m_lastClickSlotPos == slotPos &&
        now - m_lastClickTime <= GetDoubleClickTime() &&
        std::abs(mouseX - m_lastClickPosition.x) <=
        GetSystemMetrics(SM_CXDOUBLECLK) / 2 &&
        std::abs(mouseY - m_lastClickPosition.y) <=
        GetSystemMetrics(SM_CYDOUBLECLK) / 2;

    if (doubleClick)
    {
        // 세 번째 클릭을 또 더블클릭으로 처리하지 않는다.
        m_lastClickValid = false;
        return true;
    }

    m_lastClickValid = true;
    m_lastClickInventory = inventory;
    m_lastClickKey = itemKey;
    m_lastClickInventoryType = inventoryType;
    m_lastClickSlotPos = slotPos;
    m_lastClickTime = now;
    m_lastClickPosition = { mouseX, mouseY };

    return false;
}

void ShopUI::HandleRMouseClick(int mouseX, int mouseY)
{
    m_lastClickValid = false;

    if (m_isShopDragging)
        return;

    // 기존 선택이 아니라 우클릭한 행을 대상으로 처리한다.
    if (HandleProductClick(mouseX, mouseY))
    {
        ShopPacketHandler::SendBuy(m_selectedProductId, 1);
        return;
    }

    if (HandleInventoryRowClick(mouseX, mouseY))
    {
        ShopPacketHandler::SendSell(m_selectedSellInventoryType, m_selectedSellSlotPos,  1);
    }
}

void ShopUI::HandleDragging(int mouseX, int mouseY)
{
    if (!m_isShopDragging)
        return;

    m_ShopImgPosX = mouseX - m_dragOffsetX;
    m_ShopImgPosY = mouseY - m_dragOffsetY;
}

void ShopUI::HandleMouseUp()
{
    m_isShopDragging = false;
}

bool ShopUI::IsPointInRect(const RECT& rect, int mouseX, int mouseY) const
{
    return mouseX >= rect.left && mouseX < rect.right &&
        mouseY >= rect.top && mouseY < rect.bottom;
}

stb::Texture* ShopUI::GetCurrentImg(const ShopButton& button) const
{
    if (!button.isEnable)
        return button.disabledImg;

    switch (button.state)
    {
    case UIButtonState::Normal:
        return button.normalImg;

    case UIButtonState::Hover:
        return button.hoverImg;

    case UIButtonState::Pressed:
        return button.pressedImg;

    default:
        return button.normalImg;
    }
}

void ShopUI::CreateProductRows()
{
    m_productRows.clear();
    m_productRows.reserve(kPageSize);

    for (int i = 0; i < kPageSize; ++i)
    {
        ShopProductRow row{};
        const LONG top = 142 + i * 50;

        row.size = { 14, top, 350, top + 44 };
        m_productRows.push_back(row);
    }
}

void ShopUI::UpdateProductRows()
{
    const auto* shop =ShopManager::getInstance()->GetCurrentShop();

    for (int i = 0; i < static_cast<int>(m_productRows.size()); ++i)
    {
        auto& row = m_productRows[i];

        row.productId = 0;
        row.itemId = 0;
        row.price = 0;
        row.isEnable = false;
        row.isHover = false;

        if (shop == nullptr)
            continue;

        const int index = m_page * kPageSize + i;

        if (index >= static_cast<int>(shop->products.size()))
            continue;

        const auto& product = shop->products[index];

        row.productId = product.productId;
        row.itemId = product.itemId;
        row.price = product.price;
        row.isEnable = true;
    }
}

void ShopUI::UpdateProductRowState(int mouseX, int mouseY)
{
    const int localX = static_cast<int>(mouseX - m_ShopImgPosX);
    const int localY = static_cast<int>(mouseY - m_ShopImgPosY);

    for (auto& row : m_productRows)
    {
        row.isHover = row.isEnable && !m_isShopDragging && IsPointInRect(row.size, localX, localY);
    }
}

bool ShopUI::HandleProductClick(int mouseX, int mouseY)
{
    const int localX = static_cast<int>(mouseX - m_ShopImgPosX);
    const int localY = static_cast<int>(mouseY - m_ShopImgPosY);

    for (const auto& row : m_productRows)
    {
        if (!row.isEnable || !IsPointInRect(row.size, localX, localY))
        {
            continue;
        }

        m_selectedProductId = row.productId;
        return true;
    }

    return false;
}


void ShopUI::CreateInventoryRows()
{
    m_inventoryRows.clear();
    m_inventoryRows.reserve(kPageSize);

    for (int i = 0; i < kPageSize; ++i)
    {
        ShopInventoryRow row{};
        const LONG top = 142 + i * 50;

        row.size = { 396, top, 674, top + 44 };
        m_inventoryRows.push_back(row);
    }
}

void ShopUI::UpdateInventoryRows()
{
    auto* player = PlayerManager::getInstance()->GetLocalPlayer();
    const auto* shop = ShopManager::getInstance()->GetCurrentShop();

    if (player == nullptr || shop == nullptr)
        return;

    auto* inventoryManager = player->GetInvenManager();

    if (inventoryManager == nullptr)
        return;

    std::vector<InventoryItemInfo> items;

    // 서버 인벤토리 종류: 장비, 소비, 기타, 설치, 캐시
    for (int type = 0; type < 5; ++type)
    {
        auto* inventory = inventoryManager->GetInventory(type);

        if (inventory == nullptr)
            continue;

        for (auto item : inventory->GetItemInfos())
        {
            if (item.itemId <= 0 || item.itemCount <= 0)
                continue;

            item.inventoryType = type;
            items.push_back(item);
        }
    }

    std::sort(items.begin(), items.end(),
        [](const InventoryItemInfo& a, const InventoryItemInfo& b)
        {
            if (a.inventoryType != b.inventoryType)
                return a.inventoryType < b.inventoryType;

            return a.slotPos < b.slotPos;
        });

    const int itemCount = static_cast<int>(items.size());
    const int lastPage = itemCount == 0 ? 0 : (itemCount - 1) / kPageSize;

    if (!m_isShopDragging)
    {
        if (M_INPUT->GetKeyDown(stb::eKeyCode::Home) &&
            m_sellPage > 0)
        {
            --m_sellPage;
            m_selectedSellItemId = 0;
            m_lastClickValid = false;
        }

        if (M_INPUT->GetKeyDown(stb::eKeyCode::End) && m_sellPage < lastPage)
        {
            ++m_sellPage;
            m_selectedSellItemId = 0;
            m_lastClickValid = false;
        }
    }

    if (m_sellPage > lastPage)
        m_sellPage = lastPage;

    bool selectedExists = false;

    for (const auto& item : items)
    {
        if (item.inventoryType == m_selectedSellInventoryType && item.slotPos == m_selectedSellSlotPos && item.itemId == m_selectedSellItemId)
        {
            selectedExists = true;
            break;
        }
    }

    if (!selectedExists)
    {
        m_selectedSellInventoryType = -1;
        m_selectedSellSlotPos = -1;
        m_selectedSellItemId = 0;
    }

    for (int i = 0; i < static_cast<int>(m_inventoryRows.size()); ++i)
    {
        auto& row = m_inventoryRows[i];
        const RECT size = row.size;

        row = {};
        row.size = size;

        const int index = m_sellPage * kPageSize + i;

        if (index >= itemCount)
            continue;

        const auto& item = items[index];

        row.inventoryType = item.inventoryType;
        row.slotPos = item.slotPos;
        row.itemId = item.itemId;
        row.itemCount = item.itemCount;
        row.isEnable = true;
    }
}

void ShopUI::UpdateInventoryRowState(int mouseX, int mouseY)
{
    const int localX = static_cast<int>(mouseX - m_ShopImgPosX);
    const int localY = static_cast<int>(mouseY - m_ShopImgPosY);

    for (auto& row : m_inventoryRows)
    {
        row.isHover = row.isEnable && !m_isShopDragging &&  IsPointInRect(row.size, localX, localY);
    }
}

bool ShopUI::HandleInventoryRowClick(int mouseX, int mouseY)
{
    const int localX = static_cast<int>(mouseX - m_ShopImgPosX);
    const int localY = static_cast<int>(mouseY - m_ShopImgPosY);

    for (const auto& row : m_inventoryRows)
    {
        if (!row.isEnable ||
            !IsPointInRect(row.size, localX, localY))
        {
            continue;
        }

        m_selectedSellInventoryType = row.inventoryType;
        m_selectedSellSlotPos = row.slotPos;
        m_selectedSellItemId = row.itemId;
        return true;
    }

    return false;
}
