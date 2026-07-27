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

	 void SetWorldSize(const Vector2& size){ mWorldSize = size;}
	 void SetLookOffset(const Vector2& offset){mLookOffset = offset;}
	 const Vector2& GetDistance() const { return mDistance;}
	private:
		class GameObject* mTarget;

		Vector2 mDistance;
		// 윈도우 크기
		Vector2 mResolution;
		// 타켓의 위치
		Vector2 mLookPosition;

		Vector2 mWorldSize = Vector2::Zero;
		Vector2 mLookOffset = Vector2::Zero;
	};
}


