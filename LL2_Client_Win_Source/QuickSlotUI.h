#pragma once
#include "UI.h"
#include "stbTexture.h"
#include "UILayout.h"
#include "stbInput.h"
#include "QuickSlotUI_Info.h"

class QuickSlotUI : public UI
{
public:

	void Init() override;
	void Update() override;
	void Render(stbD2DRenderer& renderer) override;
	void RenderTestBox(stbD2DRenderer& renderer);
	void RenderSlotItem(stbD2DRenderer& renderer);
	void RenderItemSlot(stbD2DRenderer& renderer, const QuickSlotData& slot, const UIRect& rect);
	void RenderSkillSlot(stbD2DRenderer& renderer, const QuickSlotData& slot, const UIRect& rect);
	void RednerSlotText(stbD2DRenderer& renderer);
	void CreateSlotRect();
	int GetSlotIndexByPoint(int mouseX, int mouseY);
	void HandleClickSlot(int slotIndex);


private:
	stb::Texture* m_background = nullptr;

	bool isReduce = false;

	std::vector<UIRect> m_slotRects;
	
	std::vector<stb::eKeyCode> m_quickSlotKeys;
	UIRect m_UIRect;

private:
	static constexpr float BASE_BG_WIDTH = 557.0f;
	static constexpr float BASE_BG_HEIGHT = 67.0f;

	static constexpr int SLOT_COUNT = 32;
	static constexpr int SLOT_COLS = 16;

	static constexpr float BASE_START_X = 1.0f;
	static constexpr float BASE_START_Y = 1.0f;

	static constexpr float BASE_SLOT_WIDTH = 30.0f;
	static constexpr float BASE_SLOT_HEIGHT = 30.0f;

	static constexpr float BASE_GAP_X = 5.0f;
	static constexpr float BASE_GAP_Y = 3.0f;

	float m_scale =0.0f;

	
};

