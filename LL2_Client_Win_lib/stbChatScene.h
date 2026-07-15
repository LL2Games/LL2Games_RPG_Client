#pragma once
#include "..\\LL2_Client_Win_Source\\stbScene.h"


namespace stb
{
	using namespace math;
	class ChatScene : public Scene
	{
	public:
		ChatScene();
		~ChatScene();


		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(HDC hdc) override;
		void Render(stbD2DRenderer& renderer) override;
		void OnExit() override;
		void OnEnter() override;

		// 채팅 UI 상태
		void ToggleInputFocus();           // Enter 키 등으로 토글
		//bool IsInputActive() const { return mInputActive; }
		//void AppendMessage(const std::wstring& line);


	private:
		bool mInputActive = false;
		std::wstring mInputBuffer;
		//std::vector<std::wstring> mLogLines;   // 최근 N줄

	};

}

