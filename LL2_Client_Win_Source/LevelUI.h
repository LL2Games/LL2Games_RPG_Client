#pragma once
#include "CommonInclude.h"
#include "UI.h"
#include "InventoryUI_Info.h"
#include "UILayout.h"
#include <array>


class LevelUI : public UI
{
public:
	void Init() override;
	void Update() override;
	void Render(HDC hdc) override;
	void Render(stbD2DRenderer& renderer) override;
	void RenderBackground(stbD2DRenderer& renderer);
	void RenderLevel(stbD2DRenderer& renderer);
	void RenderNickName(stbD2DRenderer& renderer);
private:
	stb::Texture* m_background = nullptr;
	stb::Texture* m_levelImg = nullptr;

	UIRect m_UIRect{};

	std::array<stb::Texture*, 10> m_numbers{};
};

