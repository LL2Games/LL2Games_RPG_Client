#pragma once
#include "stbScene.h"

namespace stb
{
	class DontDestroyOnLoad : public Scene
	{
	public:
		DontDestroyOnLoad();
		~DontDestroyOnLoad();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(HDC hdc) override;

		void OnExit() override;
		void OnEnter() override;

	private:
		class Player* mPlayer;

	};

}

