#pragma once

#include "UI.h"
#include "ShopUI_Info.h"
#include <vector>

class ShopUI : public UI
{
public:
    void Init() override;
    void Update() override;
    void Render(stbD2DRenderer& renderer) override;

    void ResetSelection();

private:
    void Init_ShopButton();

    void RenderBackground(stbD2DRenderer& renderer);
    void RenderButtons(stbD2DRenderer& renderer);
    void RenderProductRows(stbD2DRenderer& renderer);
    void RenderInventoryRows(stbD2DRenderer& renderer);
    void RenderGold(stbD2DRenderer& renderer);


    void UpdateButtonState(int mouseX, int mouseY);
    void UpdateSingleButtonState(ShopButton& button, int localX, int localY);

    void HandleLMouseClick(int mouseX, int mouseY);
    void HandleRMouseClick(int mouseX, int mouseY);
    bool HandleButtonClick(int mouseX, int mouseY);
    bool CheckDoubleClick(bool inventory, int itemKey, int inventoryType, int slotPos, int mouseX, int mouseY);

    void HandleDragging(int mouseX, int mouseY);
    void HandleMouseUp();

    bool IsPointInRect(const RECT& rect, int mouseX, int mouseY) const;

    stb::Texture* GetCurrentImg(const ShopButton& button) const;

    void CreateProductRows();
    void UpdateProductRows();
    void UpdateProductRowState(int mouseX, int mouseY);
    bool HandleProductClick(int mouseX, int mouseY);
  

    void CreateInventoryRows();
    void UpdateInventoryRows();
    void UpdateInventoryRowState(int mouseX, int mouseY);
    bool HandleInventoryRowClick(int mouseX, int mouseY);
    

private:
    float m_shopScale = 0.8f;

    float m_ShopImgPosX = 300.0f;
    float m_ShopImgPosY = 10.0f;

    bool m_isShopDragging = false;
    float m_dragOffsetX = 0.0f;
    float m_dragOffsetY = 0.0f;

    static constexpr int kPageSize = 10;
    int m_page = 0;
    int m_selectedProductId = 0;
    int m_sellPage = 0;
    int m_selectedSellInventoryType = -1;
    int m_selectedSellSlotPos = -1;
    int m_selectedSellItemId = 0;

    bool m_lastClickValid = false;
    bool m_lastClickInventory = false;

    int m_lastClickKey = 0;
    int m_lastClickInventoryType = -1;
    int m_lastClickSlotPos = -1;

    ULONGLONG m_lastClickTime = 0;
    POINT m_lastClickPosition{};

private:

    stb::Texture* m_background = nullptr;

    ShopButton m_closeButton{};

    inline static constexpr RECT kCloseRect = { 664, 6, 690, 32 };
    inline static constexpr RECT kTitleRect = { 6, 6, 656, 32 };

    std::vector<ShopProductRow> m_productRows;
    std::vector<ShopInventoryRow> m_inventoryRows;

    stb::Texture* m_productRowNormal = nullptr;
    stb::Texture* m_productRowHover = nullptr;
    stb::Texture* m_productRowSelected = nullptr;
    stb::Texture* m_inventoryRowNormal = nullptr;
    stb::Texture* m_inventoryRowHover = nullptr;
    stb::Texture* m_inventoryRowSelected = nullptr;
    stb::Texture* m_coinImg = nullptr;
};
