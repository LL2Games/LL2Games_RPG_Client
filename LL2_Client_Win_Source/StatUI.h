#pragma once

#include "UI.h"
#include "stbTexture.h"
#include "Stat.h"
#include "d2d1helper.h"
#include <array>

class Texture;
class stbD2DRenderer;

enum class StatType
{
	Str = 1,
	Dex,
	Int,
	Luk
};

struct PlusButtonInfo
{
	StatType statType;
	D2D1_POINT_2F position;
};

class StatUI : public UI
{
public:
	void Init() override;
	void Update() override;
	void Render(HDC hdc) override;
	void Render(stbD2DRenderer& renderer) override;

public:
	void HandleDragging(int mouseX, int mouseY);
	void HandleLMouseClick(int mouseX, int mouseY);
	void HandleMouseUp();
	bool HandleStatUIClick(int mouseX, int mouseY);
	bool IsPointInRect(const D2D1_RECT_F& tabRect, int mouseX, int mouseY);

	void UpdatePlayerStat();	
	void FinishStatUpRequest();
private:
	void RenderBackGround(stbD2DRenderer& renderer);
	void RenderPlusButton(stbD2DRenderer& renderer);
	void RenderStatText(stbD2DRenderer& renderer);

	D2D1_RECT_F GetPlusButtonRect(D2D1_POINT_2F buttonPos, D2D1_SIZE_F buttonSize);
	bool HandlePlusButtonClick(int mouseX, int mouseY);

private:

	int m_statImgPosX = 300;
	int m_statImgPosY = 100;

	bool m_isStatDragging = false;	// 인벤토리 창 드래그용
	int m_dragOffsetX = 0;
	int m_dragOffsetY = 0;

	D2D1_RECT_F  m_statClickRect{ 315, 118, 565, 158 };

	// UI 이동 시 사용될 클릭 가능 구역 사이즈
	static constexpr int m_statClickWidth = 250;
	static constexpr int m_statClickHeight = 40;

	static constexpr D2D1_RECT_F m_nameTextRect = {125.0f, 89.0f, 210.0f, 104.0f};
	static constexpr D2D1_RECT_F m_jobTextRect = { 125.0f, 114.0f, 210.0f, 129.0f };
	static constexpr D2D1_RECT_F m_levelTextRect = { 125.0f, 139.0f, 210.0f, 154.0f };

	static constexpr D2D1_RECT_F m_hpTextRect = { 125.0f, 175.0f, 210.0f, 190.0f };
	static constexpr D2D1_RECT_F m_mpTextRect = { 125.0f, 200.0f, 210.0f, 215.0f };

	static constexpr D2D1_RECT_F m_strTextRect = { 125.0f, 238.0f, 210.0f, 253.0f };
	static constexpr D2D1_RECT_F m_dexTextRect = { 125.0f, 263.0f, 210.0f, 278.0f };
	static constexpr D2D1_RECT_F m_intTextRect = { 125.0f, 285.0f, 210.0f, 300.0f };
	static constexpr D2D1_RECT_F m_luckTextRect = { 125.0f, 310.0f, 210.0f, 325.0f };

	static constexpr D2D1_RECT_F m_remainAPTextRect = { 125.0f, 363.0f, 210.0f, 378.0f };

	inline static constexpr std::array<PlusButtonInfo, 4>
		kPlusButtons{ {
			{ StatType::Str, { 220.0f, 235.0f } },
			{ StatType::Dex, { 220.0f, 260.0f } },
			{ StatType::Int, { 220.0f, 285.0f } },
			{ StatType::Luk, { 220.0f, 309.0f } }
		} };

private:
	stb::Texture* m_background = nullptr;
	stb::Texture* m_plusButton = nullptr;

	StatViewData m_statData{};
	D2D1_SIZE_F m_plusButtonSize{};

	bool m_isActivePlusButton = false;
		
};

