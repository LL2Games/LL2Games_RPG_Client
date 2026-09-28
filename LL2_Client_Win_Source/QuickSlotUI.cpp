#include "QuickSlotUI.h"
#include "stbTexture.h"
#include "stbD2DRenderer.h"
#include "stbResourceManager.h"
#include "UILayout.h"
#include "stbApplication.h"
#include "stbInput.h"
#include "ItemDataManager.h"
#include "QuickSlotManager.h"
#include "PlayerManager.h"
#include "InventoryManager.h"
#include <algorithm>

#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_APP stb::SingletonBase<stb::Application>::getInstance()
#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()
#define M_ITEMDATAMANAGER stb::SingletonBase<ItemDataManager>::getInstance()
#define M_QUICKSLOTMANAGER stb::SingletonBase<QuickSlotManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
#define M_INVENTORYMANAGER stb::SingletonBase<InventoryManager>::getInstance()

void QuickSlotUI::Init()
{
    m_background = M_REMANAGER->Find<stb::Texture>(L"quickslot_background");
    
}

void QuickSlotUI::Update()
{

    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(M_APP->GetHWND(), &pt);

    if (M_INPUT->GetKeyDown(stb::eKeyCode::LButton))
    {
        GetSlotIndexByPoint(pt.x, pt.y);
    }



}

void QuickSlotUI::Render(stbD2DRenderer& renderer)
{
    
    if (m_background == nullptr)
    {
        OutputDebugStringA("m_background null\n");
        return;
    }

    ID2D1Bitmap* bitmap = m_background->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("bitmap null\n");
        return;
    }

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    // 기준 해상도 대비 UI 전체 스케일
    float scaleX = rtSize.width / 1366.0f;
    float scaleY = rtSize.height / 768.0f;
    float scale = (std::min)(scaleX, scaleY);

    m_scale = scale;

    float drawWidth = BASE_BG_WIDTH * 1.0f;
    float drawHeight = BASE_BG_HEIGHT * 1.0f;

    m_UIRect = UILayout::CalcRect(
        rtSize.width,
        rtSize.height,
        drawWidth,
        drawHeight,
        UIAnchor::BottomRight,
        2.0f,
        15.0f
    );

    renderer.DrawBitmap(
        bitmap,
        static_cast<FLOAT>(m_UIRect.x),
        static_cast<FLOAT>(m_UIRect.y),
        static_cast<FLOAT>(m_UIRect.width),
        static_cast<FLOAT>(m_UIRect.height),
        0.3f
    );
    CreateSlotRect();
    //RenderTestBox(renderer);
    RenderSlotItem(renderer);
    RednerSlotText(renderer);
}

void QuickSlotUI::RenderTestBox(stbD2DRenderer& renderer)
{
    
    for (const auto& rect : m_slotRects)
    {
        renderer.DrawRect(
            (FLOAT)rect.x,
            (FLOAT)rect.y,
            (FLOAT)rect.width,
            (FLOAT)rect.height,
            D2D1::ColorF::Black
        );
    }

    renderer.DrawRect(
        (FLOAT)m_UIRect.x,
        (FLOAT)m_UIRect.y,
        (FLOAT)m_UIRect.width,
        (FLOAT)m_UIRect.height,
        D2D1::ColorF::Black
    );
}

void QuickSlotUI::RenderSlotItem(stbD2DRenderer& renderer)
{
    for (int i = 0; i < (int)m_slotRects.size(); ++i)
    {
       
        const QuickSlotData* slot = M_PLAYERMANAGER->GetLocalPlayer()->GetQuickSlotManager()->GetSlot(i);
   
        if (slot == nullptr)
            continue;

        if (slot->type == QuickSlotType::None)
        {
            continue;
        }
           
        const UIRect& rect = m_slotRects[i];

        if (slot->type == QuickSlotType::Item)
        {
            RenderItemSlot(renderer, *slot, rect);
        }
        else if (slot->type == QuickSlotType::Skill)
        {
            RenderSkillSlot(renderer, *slot, rect);
        }
    }
}

void QuickSlotUI::RenderItemSlot(stbD2DRenderer& renderer, const QuickSlotData& slot, const UIRect& rect)
{
 
    std::wstring key = std::to_wstring(slot.ref_id);

    stb::Texture* texture = M_REMANAGER->Find<stb::Texture>(key);
    if (texture == nullptr)
        return;

    ID2D1Bitmap* bitmap = texture->GetD2DBitmap();
    if (bitmap == nullptr)
        return;

    float itemWidth = static_cast<float>(texture->GetWidth());
    float itemHeight = static_cast<float>(texture->GetHeight());

    if (itemWidth > rect.width)
        itemWidth = rect.width;

    if (itemHeight > rect.height)
        itemHeight = rect.height;

    float itemX = rect.x + (rect.width - itemWidth) / 2.0f;
    float itemY = rect.y + (rect.height - itemHeight) / 2.0f;

    renderer.DrawBitmap(
        bitmap,
        itemX,
        itemY,
        itemWidth,
        itemHeight,
        1.0f
    );

   
    int itemCount = 0;

    Inventory* inventory = M_INVENTORYMANAGER->GetInventory(static_cast<int>(slot.inventory_type));

    if (inventory != nullptr)
    {
        InventoryItemInfo* item = inventory->FindSlot(slot.inventory_slotPos);

        if (item != nullptr && item->itemId == slot.ref_id)
        {
            itemCount = item->itemCount;
        }
    }

    std::wstring countText = std::to_wstring(itemCount);

    D2D1_RECT_F textRect = D2D1::RectF(
        (FLOAT)rect.x,
        (FLOAT)(rect.y + rect.height - 11.0f),
        (FLOAT)(rect.x + rect.width - 3.0f),
        (FLOAT)(rect.y + rect.height)
    );

    renderer.DrawTextString(
        countText,
        textRect,
        D2D1::ColorF::Black,
        TextStyle::QuickSlot
    );
    
}

void QuickSlotUI::RenderSkillSlot(stbD2DRenderer& /*renderer*/, const QuickSlotData& /*slot*/, const UIRect& /*rect*/)
{
}

void QuickSlotUI::RednerSlotText(stbD2DRenderer& renderer)
{
    for (int i = 0; i < (int)m_slotRects.size(); ++i)
    {
       stb::eKeyCode keyCode = M_INPUT->FindKeyByQuickSlotIndex(i);
       std::wstring str_key = M_INPUT->KeyCodeToWString(keyCode);

       D2D1_RECT_F textRect = D2D1::RectF(
           (FLOAT)m_slotRects[i].x + 2.0f,
           (FLOAT)m_slotRects[i].y + 1.0f,
           (FLOAT)m_slotRects[i].x + m_slotRects[i].width,
           (FLOAT)m_slotRects[i].y + 14.0f
       );

       renderer.DrawTextString(
           str_key,
           textRect,
           D2D1::ColorF::Black,
           TextStyle::QuickSlot
       );
    }
}

void QuickSlotUI::CreateSlotRect()
{
    m_slotRects.clear();
    m_slotRects.reserve(SLOT_COUNT);

    float ratioX = (float)m_UIRect.width / BASE_BG_WIDTH;
    float ratioY = (float)m_UIRect.height / BASE_BG_HEIGHT;

    float startX = m_UIRect.x + BASE_START_X * ratioX;
    float startY = m_UIRect.y + BASE_START_Y * ratioY;

    float slotWidth = BASE_SLOT_WIDTH * ratioX;
    float slotHeight = BASE_SLOT_HEIGHT * ratioY;

    float gapX = BASE_GAP_X * ratioX;
    float gapY = BASE_GAP_Y * ratioY;

    for (int i = 0; i < SLOT_COUNT; ++i)
    {
        int col = i % SLOT_COLS;
        int row = i / SLOT_COLS;

        float left = startX + col * (slotWidth + gapX);
        float top = startY + row * (slotHeight + gapY);
        float right = left + slotWidth;
        float bottom = top + slotHeight;

        UIRect rect;
        rect.x = roundf(left);
        rect.y = roundf(top);
        rect.width = roundf(right) - rect.x;
        rect.height = roundf(bottom) - rect.y;

        m_slotRects.push_back(rect);
    }
 }

int QuickSlotUI::GetSlotIndexByPoint(int mouseX, int mouseY)
{
    for (int i = 0; i < (int)m_slotRects.size(); ++i)
    {
        const UIRect& rect = m_slotRects[i];

        if (mouseX >= rect.x && mouseX <= rect.x + rect.width &&
            mouseY >= rect.y && mouseY <= rect.y + rect.height)
        {
            std::string DebugMsg;
            DebugMsg = std::to_string(i) + " QuickSlot Click \n";
            OutputDebugStringA(DebugMsg.c_str());
            return i;
        }
    }

    return -1;
}

void QuickSlotUI::HandleClickSlot(int /*slotIndex*/)
{
    //stb::eKeyCode key = m_quickSlotKeys[slotIndex];

    //stb::KeyBindInfo bind;
    //if (!M_INPUT->GetBindInfo(key, bind))
    //    return;
    //
    //player->ExecuteBind(bind);
}

