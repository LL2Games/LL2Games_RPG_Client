#pragma once
#include "UI.h"
#include "stbTexture.h"
#include "Trade_Info.h"
#include <d2d1.h>

#include "InventoryUI_Info.h"

class stbD2DRenderer;

class TradeUI : public UI
{
public:
	void Init() override;
	void Update() override;
	void Render(HDC hdc) override;
	void Render(stbD2DRenderer& renderer) override;


//인벤토리
private:
	void Init_InventoryTab();
	void Init_InventoryButton();
	void CreateSlots();
	void CreateTradeSlots();
	void ClearSlots();
	void ClearTradeSlots();
	void UpdateSlots();
	void UpdateSlotEnableState();
	void UpdateInventoryByType();
	std::unordered_map<int, InventoryTabButton> m_tabs;

	static constexpr RECT m_equipTabRect = { 10,30,45,52 };
	static constexpr RECT m_consumeTabRect = { 47,30,82,52 };
	static constexpr RECT m_etcTabRect = { 84,30,119,52 };
	static constexpr RECT m_setupTabRect = { 121,30,156,52 };
	static constexpr RECT m_cashTabRect = { 158, 30, 193, 52 };
	static constexpr RECT m_cosmeticTabRect = { 195, 30, 230, 52 };

	static constexpr RECT m_fullEquipTabRect = { 10,30,138,52 };
	static constexpr RECT m_fullConsumeTabRect = { 140, 30, 268, 52 };
	static constexpr RECT m_fullEtcTabRect = { 270, 30, 398, 52 };
	static constexpr RECT m_fullSetupTabRect = { 400 ,30, 528, 52 };
	static constexpr RECT m_fullCashTabRect = { 530, 30, 658, 52 };
	static constexpr RECT m_fullCosmeticTabRect = { 660, 30, 788, 52 };

	InventoryButton m_fullButton;
	InventoryButton m_minButton;
	InventoryButton m_closeButton;

	static constexpr RECT m_minimize_minButton = { 170, 8, 189,27 };
	static constexpr RECT m_minimize_fullButton = { 190, 8, 207,27 };
	static constexpr RECT m_minimize_closeButton = { 208, 4, 234,31 };

	static constexpr RECT m_full_minButton = { 720, 8, 738, 27 };
	static constexpr RECT m_full_fullButton = { 740, 8, 758, 27 };
	static constexpr RECT m_full_closeButton = { 760, 4, 778, 27 };

	InventoryType m_currentType;


	struct Slot
	{
		RECT rect;
		int itemId = 0;
		int itemCount = 0;
	};

	std::vector<Slot> mSlots;
	std::vector<InventorySlotUI> m_slots;
	std::vector<InventorySlotUI> m_tradeMySlots;
	std::vector<InventorySlotUI> m_tradeTargetSlots;

	static constexpr float m_slotStartX = 474.0f;
	static constexpr float m_slotStartY = 80.0f;
	static constexpr float m_slotWidth = 42.0f;
	static constexpr float m_slotHeight = 40.0f;
	static constexpr float m_slotgapX = 4.4f;
	static constexpr float m_slotgapY = 6.0f;

	static constexpr int m_slotCols = 4;
	static constexpr int m_fullSlotCols = 16;

	static constexpr int m_slotMaxCount = 128;
	int m_inventoryImgPosX = 680;
	int m_inventoryImgPosY = 100;

	static constexpr int m_AddInventoryMenuPosX = 450;
	static constexpr int m_AddInventoryMenuPosY = 100;
	bool m_isExpand = false;

	int m_posX = 230;
	int m_posY = 100;

	//교환 슬롯
	int m_tradeMySlotPosX = 235;
	int m_tradeMySlotPosY = 190;

	int m_tradeTargetSlotPosX = 30;
	int m_tradeTargetSlotPosY = 190;
	static constexpr int m_tradeSlotCols = 4;
	static constexpr int m_tradeSlotMaxCount = 12;

	RECT m_tradeClickRect = { 230, 100, 670, 133 };
	bool m_isDragging = false;
	static constexpr int m_tradeCanClickWidth = 440;
	static constexpr int m_tradeCanClickHeight = 32;

	static constexpr int m_fullTradeCanClickWidth = 1150;
	static constexpr int m_fullTradeCanClickHeight = 30;

	int m_dragOffsetX;
	int m_dragOffsetY;

	bool m_isItemDragging = false; // 인벤토리 내 아이템 드래그 용
	int m_dragCurrentMouseX;
	int m_dragCurrentMouseY;

	int m_dragStartSlotIndex;
	/*int m_dragCurrentMouseX;
	int m_dragCurrentMouseY;*/
	int m_dragItemId;
	int m_dragItemCount;


	std::vector<TradeSlotInfo> m_mySlotInfo;
	std::vector<TradeSlotInfo> m_targetSlotInfo;

private:
	//내 아이템 슬롯 영역(왼쪽 패널)
	void RenderMySlots(stbD2DRenderer& renderer);
	//상대 아이템 슬롯 영역
	void RenderTargetSlots(stbD2DRenderer& renderer);
	//확정/취소 버튼
	void RenderButtons(stbD2DRenderer& renderer);
	void RenderBackground(stbD2DRenderer& renderer);
	void RenderConfirmLayerMe(stbD2DRenderer& renderer);
	void RenderConfirmLayerTarget(stbD2DRenderer& renderer);
	void RenderButton(stbD2DRenderer& renderer);
	void RenderNickname(stbD2DRenderer& renderer);
	void RenderCancelPopUp(stbD2DRenderer& renderer);
	void RenderSuccessPopUp(stbD2DRenderer& renderer);

	void RenderInventoryMenuButtons(stbD2DRenderer& renderer);
	void RenderInventorySlotItem(stbD2DRenderer& renderer);
	void RenderTradeSlotItem(stbD2DRenderer& renderer);
	void RenderDraggingItem(stbD2DRenderer& renderer);
	void RenderInventoryButtons(stbD2DRenderer& renderer);
	void RenderInventoryTestSlots(stbD2DRenderer& renderer);

	void RenderTradeTestSlots(stbD2DRenderer& renderer);


	stb::Texture* GetCurrentImg(InventoryButton& buttons);

	bool IsPointInTradeReady(int mouseX, int mouseY);

	void HandleLMouseClick(int mouseX, int mouseY);
	bool HandleTradeUIDragging(int mouseX, int mouseY);
	bool HandleTabClick(int mouseX, int mouseY);
	bool HandleButtonClick(int mouseX, int mouseY);
	void HandleDragging(int mouseX, int mouseY);

	void HandleMouseUp();

	void ExpandInventory();
	void ReduceInventory();



	int GetClickedMySlotIndex(int mouseX, int mouseY);
	int GetClickedMyTradeSlotIndex(int mouseX, int mouseY);
	void CloseCancelPopup();
	void CloseSuccessPopup();



public:
	void StartTrade(const std::string& targetId, const std::string& targetName);
	void CloseTradeUI();
	void OnCancelPopUp(); //상대가 교환 취소했다는 팝업
	void OnSuccessPopUp(const std::vector<TradeSlotInfo>& mySlotInfos, const std::vector<TradeSlotInfo>& targetSlotInfos); //교환 완료 팝업
	void OnReady(); //상대 교환 준비
	void OnTargetAddItem(const TradeSlotInfo& tradeSlotInfo); //상대 아이템 추가


	bool IsQuantityInputActive() const
	{
		return m_quantityPopupActive;
	}

	void OnQuantityChar(wchar_t ch);
	void OnQuantityKeyDown(WPARAM key);


private:
	bool m_quantityPopupActive = false;
	bool m_quantityInputError = false;

	std::wstring m_quantityInput;

	int m_quantityMax = 1;
	int m_pendingItemId = 0;
	int m_pendingInvenSlot = -1;
	int m_pendingTradeSlot = -1;

	void OpenQuantityPopup(
		int itemId,
		int itemCount,
		int invenSlotIndex,
		int tradeSlotIndex);

	void CloseQuantityPopup();
	void CommitPendingTradeItem();
	void RenderQuantityPopup(stbD2DRenderer& renderer);


private:
	bool         m_cancelPopupActive = false; //상대 교환 취소 팝업용
	bool         m_successPopupActive = false; //상대 교환 완료 팝업용
	D2D1_RECT_F m_cancelCheckButtonRect{};   //교환취소 확인버튼
	D2D1_RECT_F m_successCheckButtonRect{};   //완료 확인버튼

	bool         m_ConfirmLayerMe= false; //교환 대기 레이어
	bool         m_ConfirmLayerTarget= false; //교환 대기 레이어
	
	stb::Texture* m_txtBackground = nullptr;
	stb::Texture* m_txtFullBackground = nullptr;
	stb::Texture* m_txtConfirmNormal = nullptr;
	stb::Texture* m_txtTradeNormal = nullptr;
	stb::Texture* m_txtTradeChecked = nullptr;
	stb::Texture* m_txtLayerConfirmMe = nullptr;


	//슬롯 레이아웃 (InventoryUI와 동일한 방식)
	static constexpr int SLOT_COLS = 4;
	static constexpr int SLOT_ROWS = 4;
	static constexpr int SLOT_W = 32;
	static constexpr int SLOT_H = 32;
	static constexpr int SLOT_GAP_X = 2;
	static constexpr int SLOT_GAP_Y = 2;

	static constexpr int BUTTON_CONFIRM_X = 10;
	static constexpr int BUTTON_CONFIRM_Y = 580;
	//static constexpr int BUTTON_TRADE_X = 225;
	static constexpr int BUTTON_TRADE_X = 115;
	static constexpr int BUTTON_TRADE_Y = 550;

	static constexpr int LAYER_CONFIRM_ME_X = 227;
	static constexpr int LAYER_CONFIRM_ME_Y = 43;
	static constexpr int LAYER_CONFIRM_TARGET_X = 20;
	static constexpr int LAYER_CONFIRM_TARGET_Y = 43;
	
	std::wstring m_myName;
	std::string m_myId;

	std::wstring m_targetName;
	std::string m_targetId;
};
