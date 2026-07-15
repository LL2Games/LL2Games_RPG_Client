#pragma once
#include "UI.h"
#include "UILayout.h"
#include "stbTexture.h"
#include <array>

class HealthBarUI : public UI
{
public:
	void Init() override;
	void Update() override;
	void Render(stbD2DRenderer& renderer) override;

	void RenderBackGround(stbD2DRenderer& renderer);
	void RenderHpLayer(stbD2DRenderer& renderer);
	void RenderMpLayer(stbD2DRenderer& renderer);

	void RenderHpText(stbD2DRenderer& renderer);
	void RenderMpText(stbD2DRenderer& renderer);

private:	
	stb::Texture* m_background = nullptr;
	stb::Texture* m_hpLayer = nullptr;
	stb::Texture* m_mpLayer = nullptr;
	stb::Texture* m_guage = nullptr;
	stb::Texture* m_slash = nullptr;

	UIRect m_UIRect;
private:
	std::array<stb::Texture*, 10> m_numbers{};
	UIRect m_hpBarRect{};
	UIRect m_mpBarRect{};
};

