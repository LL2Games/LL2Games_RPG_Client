#pragma once
#include "..\\LL2_Client_Win_Source\\stbScene.h"

namespace stb
{
	using namespace math;
	class PlayScene : public Scene
	{
	public:
		PlayScene();
		~PlayScene();


		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(HDC hdc) override;
		void Render(stbD2DRenderer& renderer) override;

		void OnExit() override;
		void OnEnter() override;

	private:
		class Player* mPlayer;
		class Texture* mBackground;
		class AudioClip* mBGM;
	};

}

