#pragma once
#include "CommonInclude.h"
#include "UI.h"
#include "InventoryUI_Info.h"
#include "UILayout.h"
#include <array>

class ExpBarUI : public UI
{
public:
	void Init() override;
	void Update() override;
	void Render(HDC hdc) override;
	void Render(stbD2DRenderer& renderer) override;
	void RenderBackground(stbD2DRenderer& renderer);
	void RenderGuage(stbD2DRenderer& renderer);
	void RenderExpText(stbD2DRenderer& renderer);
private:
	stb::Texture* m_background = nullptr;
	stb::Texture* m_gauge = nullptr;

private:
	std::unordered_map<int, ExpBarTextureKey> m_expBarTextureKeys;

	std::array<stb::Texture*, 10> m_numbers{};

	UIRect m_UIRect;

};

