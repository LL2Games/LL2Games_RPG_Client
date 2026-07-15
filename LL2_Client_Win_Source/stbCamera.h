#pragma once
#include "stbComponent.h"

namespace stb
{
	using namespace math;
	class Camera : public Component
	{
	public:
		Vector2 CalculatePosition(Vector2 pos) { return pos - mDistance; }

		Camera();
		~Camera();

	 void Initialize() override;
	 void Update() override;
	 void LateUpdate() override;
	 void Render(HDC hdc) override;

	 void SetTarget(GameObject* target) { mTarget = target; }

	private:
		class GameObject* mTarget;

		Vector2 mDistance;
		// 윈도우 크기
		Vector2 mResolution;
		// 타켓의 위치
		Vector2 mLookPosition;



	};
}


