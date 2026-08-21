#include "UIManager.h"
#include "stbD2DRenderer.h"
#include "InventoryUI.h"
#include "QuickSlotUI.h"
#include "HealthBarUI.h"
#include "TradeUI.h"
#include "ChatUI.h"
#include "TradeRequestUI.h"
#include "ExpBarUI.h"
#include "LevelUI.h"
#include "StatUI.h"

UIManager::UIManager()
{
	m_inventoryUI = new InventoryUI();
	m_quickslotUI = new QuickSlotUI();
	m_healthBarUI = new HealthBarUI();
	m_expBarUI = new ExpBarUI();
	m_levelUI = new LevelUI();
	m_statUI = new StatUI();
	m_tradeUI = new TradeUI();
	m_chatUI = new ChatUI();
	m_tradeReqUI = new TradeRequestUI();

}

void UIManager::Init()
{
	m_inventoryUI->Init();
	m_quickslotUI->Init();
	m_healthBarUI->Init();
	m_expBarUI->Init();
	m_levelUI->Init();
	m_statUI->Init();
	m_tradeUI->Init();
	m_chatUI->Init();
	m_tradeReqUI->Init();

	mUIs.push_back(m_inventoryUI);
	mUIs.push_back(m_quickslotUI);
	mUIs.push_back(m_healthBarUI);
	mUIs.push_back(m_expBarUI);
	mUIs.push_back(m_levelUI);
	mUIs.push_back(m_statUI);
	mUIs.push_back(m_tradeUI);
	mUIs.push_back(m_chatUI);
	mUIs.push_back(m_tradeReqUI);



	char msg[128];
	sprintf_s(msg, "UI count = %zu\n", mUIs.size());
	OutputDebugStringA(msg);
}

void UIManager::Update()
{
	for (UI* ui : mUIs)
	{
		if (ui == nullptr || ui->IsActive() == false)
			continue;

		ui->Update();
	}
}

void UIManager::Render(HDC /*hdc*/)
{

}

void UIManager::Render(stbD2DRenderer& renderer)
{

	for (UI* ui : mUIs)
	{
		//if (ui == nullptr || ui->IsActive() == false)
		if (ui == nullptr)
			continue;

		ui->Render(renderer);
	}
}

void UIManager::ToggleInventory()
{
	if (m_inventoryUI == nullptr)
		return;

	m_inventoryUI->Toggle();

	m_inventoryUI->UpdateInventoryByType();
}

void UIManager::ToggleStat()
{
	if (m_statUI == nullptr)
		return;

	m_statUI->Toggle();

	if (m_statUI->IsActive())
		m_statUI->UpdatePlayerStat();
}

#if 1
void UIManager::OpenTradeUI(const std::string& targetId, const std::string& targetName)
{
	m_tradeUI->SetActivce(true);
	m_tradeUI->StartTrade(targetId, targetName);
}
void UIManager::CloseTradeUI()
{
	m_tradeUI->CloseTradeUI();
}

void UIManager::ShowCancelPopUp()
{
	m_tradeUI->OnCancelPopUp();
}

void UIManager::ShowTradeFailPopUp(const std::string& errorMessage)
{
	if (m_tradeUI == nullptr)
		return;

	m_tradeUI->OnTradeFailPopUp(errorMessage);
}

void UIManager::ShowSuccessPopUp(const std::vector<TradeSlotInfo>& mySlotInfos, const std::vector<TradeSlotInfo>& targetSlotInfos)
{
	m_tradeUI->OnSuccessPopUp(mySlotInfos, targetSlotInfos);
}

void UIManager::TradeReadyTarget()
{
	m_tradeUI->OnReady();
}

void UIManager::ToggleTradeUI()
{
	if (m_tradeUI != nullptr)
		m_tradeUI->Toggle();
}

void UIManager::OnTradeAddItem(const TradeSlotInfo& tradeSlotInfo)
{
	if (m_tradeUI) m_tradeUI->OnTargetAddItem(tradeSlotInfo);
}

void UIManager::OpenReqTradeUI()
{
	if (m_tradeReqUI != nullptr)
		m_tradeReqUI->SetActivce(true);
}

void UIManager::CloseReqTradeUI()
{
	if (m_tradeReqUI != nullptr)
		m_tradeReqUI->CloseWindow();
		//m_tradeReqUI->SetActivce(false);
}

void UIManager::AppendInputChar_Trade(wchar_t ch)
{
	if (m_tradeReqUI) m_tradeReqUI->OnChar(ch);
}

void UIManager::HandleBackspace_Trade()
{
	if (m_tradeReqUI) m_tradeReqUI->Backspace();
}

void UIManager::KeyDownTrade(WPARAM key)
{
	if (m_tradeReqUI) m_tradeReqUI->OnKeyDown(key);
}

void UIManager::ShowTradeRequestPopUp(const TradeRequestInfo& info)
{
	if (m_tradeReqUI) m_tradeReqUI->OnPopUp(info);
}

#endif

#if 1 
void UIManager::ToggleChatInput()
{
	if (m_chatUI) m_chatUI->ToggleInputFocus();
}
bool UIManager::IsInputFocused() const
{
	return m_chatUI && m_chatUI->IsInputActive();
}

bool UIManager::IsGameplayInputBlocked() const
{
	return IsInputFocused() || IsTradeRequestActive() || IsTradeQuantityInputActive();
}

void UIManager::AppendInputChar(wchar_t ch)
{
	if (m_chatUI) m_chatUI->AppendChar(ch);
}
void UIManager::HandleBackspace()
{
	if (m_chatUI) m_chatUI->Backspace();
}
void UIManager::AppendChatMessage(const std::wstring& nick, const std::wstring& msg)
{
	if (m_chatUI) m_chatUI->AddMessage(nick, msg);
}

void UIManager::SubmitChatInput()
{
	if (m_chatUI) m_chatUI->SubmitInput();
}
#endif 


bool UIManager::IsTradeRequestActive() const
{
	return m_tradeReqUI &&
		m_tradeReqUI->IsActive();
}

bool UIManager::IsTradeQuantityInputActive() const
{
	return m_tradeUI &&
		m_tradeUI->IsQuantityInputActive();
}

void UIManager::AppendTradeQuantityChar(wchar_t ch)
{
	if (m_tradeUI)
		m_tradeUI->OnQuantityChar(ch);
}

void UIManager::KeyDownTradeQuantity(WPARAM key)
{
	if (!m_tradeUI)
		return;

	if (key == VK_RETURN)
		m_tradeQuantityEnterConsumed = true;

	m_tradeUI->OnQuantityKeyDown(key);
}

bool UIManager::ConsumeTradeQuantityEnter()
{
	if (!m_tradeQuantityEnterConsumed)
		return false;

	m_tradeQuantityEnterConsumed = false;
	return true;
}

void UIManager::RefreshInventoryUI()
{
	if (m_inventoryUI == nullptr)
		return;

	m_inventoryUI->UpdateInventoryByType();
}

void UIManager::RefreshStatUI()
{
	if (m_statUI == nullptr)
		return;

	m_statUI->UpdatePlayerStat();
	m_statUI->FinishStatUpRequest();
}
void UIManager::FinishStatUpRequest()
{
	if (m_statUI == nullptr)
		return;

	m_statUI->FinishStatUpRequest();
}


void UIManager::SetChatComposition(const std::wstring& text)
{
	if (m_chatUI)
		m_chatUI->SetComposition(text);
}

void UIManager::CommitChatComposition(const std::wstring& text)
{
	if (m_chatUI)
		m_chatUI->CommitComposition(text);
}

void UIManager::ClearChatComposition()
{
	if (m_chatUI)
		m_chatUI->ClearComposition();
}
