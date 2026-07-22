#include "InventoryUI.h"
#include "stbResourceManager.h"
#include "stbTexture.h"
#include "stbD2DRenderer.h"
#include "stbInput.h"
#include "stbApplication.h"
#include "InventoryManager.h"
#include "ItemDataManager.h"
#include "ItemPacketHandler.h"
#include "InventoryPacketHandler.h"
#include "UIManager.h"
#include "QuickSlotUI.h"
#include "QuickSlotManager.h"
#include "PlayerManager.h"

#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()
#define M_APP stb::SingletonBase<stb::Application>::getInstance()
#define M_INVENTORYMANAGER stb::SingletonBase<InventoryManager>::getInstance()
#define M_ITEMDATAMANAGER stb::SingletonBase<ItemDataManager>::getInstance()
#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()
#define M_QUICKSLOTMANAGER stb::SingletonBase<QuickSlotManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()

void InventoryUI::Init()
{
    m_background        = M_REMANAGER->Find<stb::Texture>(L"Inventory_normal");
    m_fullBackground    = M_REMANAGER->Find<stb::Texture>(L"Inventory_full");

    Init_InventoryButton();
    Init_InventoryTab();
   
    // 슬롯 생성
    CreateSlots();

    UpdateInventoryByType();

    mActive = false;
}

void InventoryUI::Init_InventoryTab()
{
    m_tabs[(int)InventoryType::Equip] =
    {
        InventoryType::Equip,
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_equip_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_equip_selected"),
            m_equipTabRect
        },
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_equip_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_equip_selected"),
            m_fullEquipTabRect
        }
    };

    m_tabs[(int)InventoryType::Consume] =
    {
        InventoryType::Consume,
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_consume_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_consume_selected"),
            m_consumeTabRect
        },
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_consume_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_consume_selected"),
            m_fullConsumeTabRect
        }
    };

    m_tabs[(int)InventoryType::Etc] =
    {
        InventoryType::Etc,
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_etc_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_etc_selected"),
            m_etcTabRect
        },
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_etc_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_etc_selected"),
            m_fullEtcTabRect
        }
    };


    m_tabs[(int)InventoryType::Setup] =
    {
        InventoryType::Setup,
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_setup_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_setup_selected"),
            m_setupTabRect
        },
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_setup_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_setup_selected"),
            m_fullSetupTabRect
        }
    };


    m_tabs[(int)InventoryType::Cash] =
    {
        InventoryType::Cash,
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_cash_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_cash_selected"),
            m_cashTabRect
        },
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_cash_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_cash_selected"),
            m_fullCashTabRect
        }
    };

    m_tabs[(int)InventoryType::Cosmetic] =
    {
        InventoryType::Cosmetic,
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_cosmetic_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_cosmetic_selected"),
            m_cosmeticTabRect
        },
        {
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_cosmetic_normal"),
            M_REMANAGER->Find<stb::Texture>(L"Inventory_full_cosmetic_selected"),
            m_fullCosmeticTabRect
        }
    };

}

void InventoryUI::Init_InventoryButton()
{
    m_fullButton = { InventoryButtonType::Expand, UIButtonState::Normal,
                    m_minimize_fullButton,
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_full_normal"),
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_full_mouseOver"),
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_full_pressed"),
                    TRUE,
                    TRUE };

    m_minButton = { InventoryButtonType::Reduce, UIButtonState::Normal,
                    m_minimize_minButton,
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_min_normal"),
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_min_mouseOver"),
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_min_pressed"),
                    TRUE,
                    TRUE };

    m_closeButton = { InventoryButtonType::Close, UIButtonState::Normal,
                    m_minimize_closeButton,
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_close_normal"),
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_close_mouseOver"),
                    M_REMANAGER->Find<stb::Texture>(L"Inventory_close_pressed"),
                    TRUE,
                    TRUE };

    m_currentType = InventoryType::Equip;
    OutputDebugStringA("Init_InventoryButton Success \n");
}

void InventoryUI::Update()
{
    if (!mActive)
        return;

    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(M_APP->GetHWND(), &pt);

    UpdateButtonState(pt.x, pt.y);

    if (M_INPUT->GetKeyDown(stb::eKeyCode::LButton))
    {
        HandleLMouseClick(pt.x, pt.y);
    }

    if (M_INPUT->GetKey(stb::eKeyCode::LButton))
    {
        HandleDragging(pt.x, pt.y);
    }

    if (M_INPUT->GetKeyUp(stb::eKeyCode::LButton))
    {
        HandleMouseUp();
    }

    if (M_INPUT->GetKeyDown(stb::eKeyCode::RButton))
    {
        HandleRMouseClick(pt.x, pt.y);
    }
}

void InventoryUI::Render(HDC /*hdc*/)
{

}

void InventoryUI::Render(stbD2DRenderer& renderer)
{
    if (!mActive)
        return;
    // 인벤토리 기본 이미지 그리기
    RenderBackGround(renderer);

    // 인벤토리 메뉴 버튼 이미지 그리기
    RenderMenuButtons(renderer);

    // 인벤토리 아이템 이미지 그리기
    RenderSlotItem(renderer);

    // 인벤토리 버튼 이미지 그리기
    RenderButtons(renderer);

    //테스트용 슬롯들 테두리 그리기
    for (const auto& slot : m_slots)
    {
        if (slot.isEnabled)
        {
            renderer.DrawRect(
                (float)slot.x,
                (float)slot.y,
                (float)slot.width,
                (float)slot.height,
                D2D1::ColorF::Black
            );
        }
        
    }

   // 테스트용 인벤토리 드래그 영역 그리기
   renderer.DrawRect(
     (float)m_inventoryClickRect.left,
     (float)m_inventoryClickRect.top,
     (float)(m_inventoryClickRect.right - m_inventoryClickRect.left),
     (float)(m_inventoryClickRect.bottom - m_inventoryClickRect.top),
     D2D1::ColorF::Black
    );

     
}

void InventoryUI::RenderSlotItem(stbD2DRenderer& renderer)
{
    for (const auto& slot : m_slots)
    {
        if (!slot.isEnabled)
            continue;

        if (slot.itemId == 0)
            continue;

        std::wstring key = std::to_wstring(slot.itemId);
        stb::Texture* texture = M_REMANAGER->Find<stb::Texture>(key);
        if (texture == nullptr)
        {
            OutputDebugStringA("texture is null\n");
            continue;
        }
            

        ID2D1Bitmap* bitmap = texture->GetD2DBitmap();
        if (bitmap == nullptr)
            continue;

        float itemWidth = static_cast<float>(texture->GetWidth());
        float itemHeight = static_cast<float>(texture->GetHeight());

        if (itemWidth > slot.width)
            itemWidth = slot.width;

        if (itemHeight > slot.height)
            itemHeight = slot.height;

        float itemX = slot.x + (slot.width - itemWidth) / 2.0f;
        float itemY = slot.y + (slot.height - itemHeight) / 2.0f;

        renderer.DrawBitmap(
            bitmap,
            itemX,
            itemY,
            itemWidth,
            itemHeight,
            1.0f
        );

      
        std::wstring countText = std::to_wstring(slot.itemCount);

        D2D1_RECT_F textRect = D2D1::RectF(
            slot.x,
            slot.y + slot.height - 13.0f,
            slot.x + slot.width - 5.0f,
            slot.y + slot.height
        );

        renderer.DrawTextString(
            countText,
            textRect,
            D2D1::ColorF::Black,
            TextStyle::Small
        );
    }

    RenderDraggingItem(renderer);
}

void InventoryUI::RenderDraggingItem(stbD2DRenderer& renderer)
{
    if (!m_isItemDragging)
        return;

    if (m_dragItemId == 0)
        return;

    
    std::wstring key = std::to_wstring(m_dragItemId);
    stb::Texture* texture = M_REMANAGER->Find<stb::Texture>(key);
    if (texture == nullptr)
        return;

    ID2D1Bitmap* bitmap = texture->GetD2DBitmap();
    if (bitmap == nullptr)
        return;

    float drawX = (float)m_dragCurrentMouseX - m_slotWidth * 0.5f;
    float drawY = (float)m_dragCurrentMouseY - m_slotHeight * 0.5f;

    renderer.DrawBitmap(
        bitmap,
        drawX,
        drawY,
        static_cast<float>(texture->GetWidth()),
        static_cast<float>(texture->GetHeight()),
        0.8f
    );
}

void InventoryUI::UpdateInventoryByType()
{
    
    Inventory* inventory = M_INVENTORYMANAGER->GetInventory((int)m_currentType);
    if (inventory == nullptr)
        return;

    const std::vector<InventoryItemInfo>& items = inventory->GetItemInfos();

    std::string DebugMsg;

    DebugMsg = "Inventory Type : " + std::to_string((int)m_currentType) + "\n";

    OutputDebugStringA(DebugMsg.c_str());

    ClearSlots();
    for (const auto& item : items)
    {
        int slotIndex = item.slotPos;

        if (slotIndex < 0 || slotIndex >= (int)m_slots.size())
            continue;
      
        m_slots[slotIndex].itemId = item.itemId;
        m_slots[slotIndex].itemCount = item.itemCount;
        
    }
}

void InventoryUI::RenderBackGround(stbD2DRenderer& renderer)
{
   
    stb::Texture* background = m_isExpand ? m_fullBackground : m_background;
    if (background == nullptr)
    {
        OutputDebugStringA("mBackground null\n");
        return;
    }

    ID2D1Bitmap* bitmap = background->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("bitmap null\n");
        return;
    }

    D2D1_SIZE_F size = bitmap->GetSize();

    renderer.DrawBitmap(bitmap,
        (FLOAT)m_inventoryImgPosX,
        (FLOAT)m_inventoryImgPosY,
        size.width,
        size.height,
        1.0f);
}

void InventoryUI::RenderMenuButtons(stbD2DRenderer& renderer)
{
    for (int i = 0; i < (int)InventoryType::EnumEnd; i++)
    {
        const InventoryTabButton& tab = m_tabs[i];
        const InventoryTabVisual& view = m_isExpand ? tab.fullView : tab.normalView;

        stb::Texture* img = (m_currentType == tab.type) ? view.selected : view.normal;
        if (img == nullptr)
            continue;

        ID2D1Bitmap* bitmap = img->GetD2DBitmap();
        if (bitmap == nullptr)
            continue;

        D2D1_SIZE_F size = bitmap->GetSize();

        renderer.DrawBitmap(
            bitmap,
            (FLOAT)m_inventoryImgPosX + view.rect.left,
            (FLOAT)m_inventoryImgPosY + view.rect.top,
            size.width,
            size.height,
            1.0f
        );
    }
}

void InventoryUI::RenderButtons(stbD2DRenderer& renderer)
{
    // 확장 이미지
    stb::Texture* full_button = GetCurrentImg(m_fullButton);
    if (full_button == nullptr)
    {
        OutputDebugStringA("full_button null\n");
        return;
    }

    ID2D1Bitmap* bitmap = full_button->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("full_button bitmap null\n");
        return;
    }

    D2D1_SIZE_F size = bitmap->GetSize();

    renderer.DrawBitmap(bitmap,
        (FLOAT)m_inventoryImgPosX + m_fullButton.size.left,
        (FLOAT)m_inventoryImgPosY + m_fullButton.size.top,
        size.width,
        size.height,
        1.0f);


    // 확장 이미지
    stb::Texture* min_button = GetCurrentImg(m_minButton);
    if (min_button == nullptr)
    {
        OutputDebugStringA("min_button null\n");
        return;
    }

    bitmap = min_button->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("min_button bitmap null\n");
        return;
    }

    size = bitmap->GetSize();

    renderer.DrawBitmap(bitmap,
        (FLOAT)m_inventoryImgPosX + m_minButton.size.left,
        (FLOAT)m_inventoryImgPosY + m_minButton.size.top,
        size.width,
        size.height,
        1.0f);


    // 닫기 이미지
    stb::Texture* close_button = GetCurrentImg(m_closeButton);
    if (close_button == nullptr)
    {
        OutputDebugStringA("close_button null\n");
        return;
    }

    bitmap = close_button->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("close_button bitmap null\n");
        return;
    }

    size = bitmap->GetSize();

    renderer.DrawBitmap(bitmap,
        (FLOAT)m_inventoryImgPosX + m_closeButton.size.left,
        (FLOAT)m_inventoryImgPosY + m_closeButton.size.top,
        size.width,
        size.height,
        1.0f);
}

void InventoryUI::CreateSlots()
{
   mSlots.clear();

   for (int index = 0; index < m_slotMaxCount; ++index)
   {
       int col = index % m_slotCols;
       int row = index / m_slotCols;

       InventorySlotUI slot;
       slot.slotIndex = index;
       slot.x = m_inventoryImgPosX + m_slotStartX + col * (m_slotWidth + m_slotgapX);
       slot.y = m_inventoryImgPosY + m_slotStartY + row * (m_slotHeight + m_slotgapY);
       slot.width = m_slotWidth;
       slot.height = m_slotHeight;
       slot.itemId = 0;
       m_slots.push_back(slot);
   }

   UpdateSlotEnableState();
}

void InventoryUI::UpdateSlots()
{
    int slotCols = m_isExpand ? m_fullSlotCols : m_slotCols;
    
    for (auto& slots : m_slots)
    {
        int col = slots.slotIndex % slotCols;
        int row = slots.slotIndex / slotCols;

        slots.x = m_inventoryImgPosX + m_slotStartX + col * (m_slotWidth + m_slotgapX);
        slots.y = m_inventoryImgPosY + m_slotStartY + row * (m_slotHeight + m_slotgapY);
    }
}

void InventoryUI::ClearSlots()
{
    for (auto& slot : m_slots)
    {
        slot.itemId = 0;
        slot.itemCount = 0;
    }
}

void InventoryUI::HandleLMouseClick(int mouseX, int mouseY)
{
    if (HandleInventoryClick(mouseX, mouseY))
        return;

    if (HandleTabClick(mouseX, mouseY))
        return;

    if (HandleButtonClick(mouseX, mouseY))
        return;

    int slotIndex = GetClickedSlotIndex(mouseX, mouseY);
    if (slotIndex != -1)
    {
        std::string msg = std::to_string(slotIndex) + "\n";
        OutputDebugStringA(msg.c_str());

        InventorySlotUI& slot = m_slots[slotIndex];

        if (slot.itemId != 0)
        {
            m_isItemDragging = true;
            m_dragStartSlotIndex = slotIndex;
            m_dragCurrentMouseX = mouseX;
            m_dragCurrentMouseY = mouseY;

            m_dragItemId = slot.itemId;
            m_dragItemCount = slot.itemCount;
        }

        return;
    }
}

void InventoryUI::HandleRMouseClick(int mouseX, int mouseY)
{
    int slotIndex = GetClickedSlotIndex(mouseX, mouseY);
    if (slotIndex != -1)
    {
        std::string msg = std::to_string(slotIndex) + "\n";
        OutputDebugStringA(msg.c_str());

        Inventory* inven = M_INVENTORYMANAGER->GetInventory(static_cast<int>(m_currentType));

        InventoryItemInfo* itemInfo = inven->FindSlot(slotIndex);
        if (itemInfo == nullptr)
            return;
        
        itemInfo->useCount = 1;

        ItemPacketHandler::SendUseItem(itemInfo);
    }
}

bool InventoryUI::HandleTabClick(int mouseX, int mouseY)
{
    int localX = mouseX - m_inventoryImgPosX;
    int localY = mouseY - m_inventoryImgPosY;

    for (int i = 0; i < (int)InventoryType::EnumEnd; i++)
    {
        const InventoryTabButton& tab = m_tabs[i];
        const InventoryTabVisual& view = m_isExpand ? tab.fullView : tab.normalView;

        if (IsPointInRect(view.rect, localX, localY))
        {
            m_currentType = tab.type;
            UpdateInventoryByType(); 
            return true;
        }    
    }
    return false;
}

bool InventoryUI::HandleButtonClick(int mouseX, int mouseY)
{
    int localX = mouseX - m_inventoryImgPosX;
    int localY = mouseY - m_inventoryImgPosY;

    if (IsPointInRect(m_fullButton.size, localX, localY))
    {
        OutputDebugStringA("fullButton Clicked \n");
        ExpandInventory();
        // 인벤토리 확장 함수 추가 필요
        return true;
    }

    if (IsPointInRect(m_minButton.size, localX, localY))
    {
        OutputDebugStringA("minButton Clicked \n");
        ReduceInventory();
        // 인벤토리 확장 함수 추가 필요
        return true;
    }


    if (IsPointInRect(m_closeButton.size, localX, localY))
    {
        OutputDebugStringA("m_closeButton Clicked \n");
        SetActivce(false);
        return true;
    }

    return false;
}

bool InventoryUI::HandleInventoryClick(int mouseX, int mouseY)
{
    int localX = mouseX - m_inventoryImgPosX;
    int localY = mouseY - m_inventoryImgPosY;

    if (IsPointInRect(m_inventoryClickRect, mouseX, mouseY))
    {
        m_isInventoryDragging = true;
        m_dragOffsetX = localX;
        m_dragOffsetY = localY;
        return true;
    }
    return false;
}

void InventoryUI::UpdateButtonState(int mouseX, int mouseY)
{
    int localX = mouseX - m_inventoryImgPosX;
    int localY = mouseY - m_inventoryImgPosY;

    UpdateSingleButtonState(m_fullButton, localX, localY);
    UpdateSingleButtonState(m_minButton, localX, localY);
    UpdateSingleButtonState(m_closeButton, localX, localY);
}

void InventoryUI::UpdateSlotEnableState()
{
    int visibleSlotCount = m_isExpand ? 128 : 32; 

    OutputDebugStringA(std::to_string(visibleSlotCount).c_str());
    OutputDebugStringA("\n");
    for (int i = 0; i < (int)m_slots.size(); ++i)
    {
        m_slots[i].isEnabled = (i < visibleSlotCount);
    }
}

void InventoryUI::UpdateSingleButtonState(InventoryButton& button, int localX, int localY)
{
    if (!button.isVisble || !button.isEnable)
    {
        button.state = UIButtonState::Normal;
        return;
    }

    bool isHover = IsPointInRect(button.size, localX, localY);

    if (!isHover)
    {
        button.state = UIButtonState::Normal;
        return;
    }

    if (M_INPUT->GetKey(stb::eKeyCode::LButton))
    {
        button.state = UIButtonState::Pressed;
    }
    else
    {
        button.state = UIButtonState::Hover;
    }
}

int InventoryUI::GetClickedSlotIndex(int mouseX, int mouseY)
{
    for (const InventorySlotUI & slot : m_slots)
    {
        if (!slot.isEnabled)
            continue;

        if (IsPointInSlot(slot, mouseX, mouseY))
        {
            return slot.slotIndex;
        }
    }

    return -1;
}

void InventoryUI::HandleMouseUp()
{
    if (m_isItemDragging)
    {
        int dropSlotIndex = GetClickedSlotIndex(m_dragCurrentMouseX, m_dragCurrentMouseY);

        if (dropSlotIndex != -1 && dropSlotIndex != m_dragStartSlotIndex)
        {
            // 여기서 서버에 패킷을 보내고 결과값을 바탕으로 아이템 위치를 변경하는 것이 맞다
            InventoryPacketHandler::SendMoveItem(static_cast<int>(m_currentType), m_dragStartSlotIndex, dropSlotIndex);
        }


        int quickSlotIndex = M_UIMANAGER->GetQuickSlotUI()->GetSlotIndexByPoint(m_dragCurrentMouseX, m_dragCurrentMouseY);

        if (quickSlotIndex != -1)
        {
            QuickSlotData data;
            data.slot_index = quickSlotIndex;
            data.type = QuickSlotType::Item;
            data.ref_id = m_dragItemId;
            data.inventory_type = m_currentType;
            data.inventory_slotPos = m_dragStartSlotIndex;
            data.count = m_dragItemCount;

            //M_PLAYERMANAGER->GetLocalPlayer()->GetQuickSlotManager()->RequestSetSlot(data);
            M_QUICKSLOTMANAGER->RequestSetSlot(data);
        }


        m_isItemDragging = false;
        m_dragStartSlotIndex = -1;
        m_dragItemId = 0;
        m_dragItemCount = 0;
        return;
    }

    m_isInventoryDragging = false;
    UpdateSlots();
}

void InventoryUI::HandleDragging(int mouseX, int mouseY)
{
    if (m_isItemDragging)
    {
        m_dragCurrentMouseX = mouseX;
        m_dragCurrentMouseY = mouseY;
        return;
    }

    if (!m_isInventoryDragging)
        return;

    int inventoryClickWidth = m_isExpand ? m_fullInventoryClickWidth : m_inventoryClickWidth;
    int inventoryClickHeight = m_isExpand ? m_fullInventoryClickHeight : m_inventoryClickHeight;

    m_inventoryImgPosX = mouseX - m_dragOffsetX;
    m_inventoryImgPosY = mouseY - m_dragOffsetY;

    m_inventoryClickRect.left = m_inventoryImgPosX;
    m_inventoryClickRect.top = m_inventoryImgPosY;
    m_inventoryClickRect.right = m_inventoryImgPosX + inventoryClickWidth;
    m_inventoryClickRect.bottom = m_inventoryImgPosY + inventoryClickHeight;

}

void InventoryUI::ExpandInventory() 
{

    m_isExpand = true;
    m_fullButton.size = m_full_fullButton;
    m_minButton.size  = m_full_minButton;
    m_closeButton.size = m_full_closeButton;

    int inventoryClickWidth = m_isExpand ? m_fullInventoryClickWidth : m_inventoryClickWidth;
    int inventoryClickHeight = m_isExpand ? m_fullInventoryClickHeight : m_inventoryClickHeight;

    m_inventoryClickRect.left = m_inventoryImgPosX;
    m_inventoryClickRect.top = m_inventoryImgPosY;
    m_inventoryClickRect.right = m_inventoryImgPosX + inventoryClickWidth;
    m_inventoryClickRect.bottom = m_inventoryImgPosY + inventoryClickHeight;

    UpdateSlotEnableState();

}

void InventoryUI::ReduceInventory()
{
    m_isExpand = false;
    m_fullButton.size   = m_minimize_fullButton;
    m_minButton.size    = m_minimize_minButton;
    m_closeButton.size  = m_minimize_closeButton;
  

    int inventoryClickWidth = m_isExpand ? m_fullInventoryClickWidth : m_inventoryClickWidth;
    int inventoryClickHeight = m_isExpand ? m_fullInventoryClickHeight : m_inventoryClickHeight;

    m_inventoryClickRect.left = m_inventoryImgPosX;
    m_inventoryClickRect.top = m_inventoryImgPosY;
    m_inventoryClickRect.right = m_inventoryImgPosX + inventoryClickWidth;
    m_inventoryClickRect.bottom = m_inventoryImgPosY + inventoryClickHeight;

    UpdateSlotEnableState();
}

stb::Texture* InventoryUI::GetCurrentImg(InventoryButton& buttons)
{
    switch (buttons.state)
    {
        case UIButtonState::Normal:
            return buttons.nomalImg;
        case UIButtonState::Hover:
            return buttons.hoverImg;
        case UIButtonState::Pressed:
            return buttons.pressedImg;
        default:
            return buttons.nomalImg;
    }
}

bool InventoryUI::IsPointInSlot(const InventorySlotUI& slot, int mouseX, int mouseY)
{
    return  mouseX >= slot.x &&
            mouseX < slot.x + slot.width &&
            mouseY >= slot.y &&
            mouseY < slot.y + slot.height;
}

bool InventoryUI::IsPointInRect(const RECT& tabRect, int mouseX, int mouseY)
{

    return  mouseX >= tabRect.left &&
        mouseX < tabRect.right &&
        mouseY >= tabRect.top &&
        mouseY < tabRect.bottom;
}

