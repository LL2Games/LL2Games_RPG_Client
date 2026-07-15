#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "stbMath.h"
#include "Trade_Info.h"


class UI;
class InventoryUI;
class QuickSlotUI;
class HealthBarUI;
class stbD2DRenderer;
class TradeUI;
class ChatUI;
class TradeRequestUI;
class ExpBarUI;
class LevelUI;

class UIManager : public stb::SingletonBase<UIManager>
{
public:
	UIManager();
	void Init();
	void Update();
	void Render(HDC hdc);
	void Render(stbD2DRenderer& renderer);

	void ToggleInventory();

public:
	//교환
	void ToggleTradeUI(); //test
	void OpenTradeUI(const std::string& targetId, const std::string& targetName);
	void OpenReqTradeUI();
	void CloseReqTradeUI();
	void AppendInputChar_Trade(wchar_t ch);
	void HandleBackspace_Trade();
	void KeyDownTrade(WPARAM key);
	void CloseTradeUI();
	void OnTradeAddItem(const TradeSlotInfo& tradeSlotInfo); //상대가 아이템 추가

	void ShowTradeRequestPopUp(const TradeRequestInfo& info); //신청 팝업
	void ShowCancelPopUp(); //상대가 교환 취소했다는 팝업
	void TradeReadyTarget(); //상대가 교환 준비
	void ShowSuccessPopUp(const std::vector<TradeSlotInfo>&, const std::vector<TradeSlotInfo>&); //교환 완료 팝업

	//채팅
	void ToggleChatInput();
	bool IsInputFocused() const;
	void AppendInputChar(wchar_t ch);
	void HandleBackspace();
	void AppendChatMessage(const std::wstring& nick, const std::wstring& msg);
	void SubmitChatInput();

	bool IsTradeRequestActive() const;

	bool ConsumeTradeQuantityEnter();
	bool IsTradeQuantityInputActive() const;
	void AppendTradeQuantityChar(wchar_t ch);
	void KeyDownTradeQuantity(WPARAM key);

	void RefreshInventoryUI();

public:
	InventoryUI* GetInventoryUI() { return m_inventoryUI; }
	QuickSlotUI* GetQuickSlotUI() { return m_quickslotUI; }
	ExpBarUI* GetExpBarUI() { return m_expBarUI; }
private:
	std::vector<UI*> mUIs;
	InventoryUI* m_inventoryUI = nullptr;
	QuickSlotUI* m_quickslotUI = nullptr;
	HealthBarUI* m_healthBarUI = nullptr;
	ExpBarUI* m_expBarUI = nullptr;
	LevelUI* m_levelUI = nullptr;

	TradeUI* m_tradeUI = nullptr; //교환
	TradeRequestUI* m_tradeReqUI = nullptr; //교환
	ChatUI* m_chatUI = nullptr; //채팅
	bool m_tradeQuantityEnterConsumed = false;
};
