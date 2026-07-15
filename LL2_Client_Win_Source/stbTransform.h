#pragma once
#include "stbEntity.h"
#include "stbComponent.h"

namespace stb
{
	using namespace math;

	class Transform : public Component
	{
	public:
		Transform();
		~Transform();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(HDC hdc) override;

		void SetPosition(Vector2 pos) { mPosition.x = pos.x; mPosition.y = pos.y; }
		Vector2 GetPosition() { return mPosition; }

		void SetScale(Vector2 scale) { mScale = scale; }
		Vector2 GetScale() {return mScale; }
			
		void SetRotation(float rotate) { mRotation = rotate; }
		float GetRotation() { return mRotation; }


	private:
		// 게임 오브젝트의 위치를 나타내기 위해서는 위치를 알아야 한다. 
		// 2D 게임이기 때문에 x,y 만 알면 된다.
		// 위치
		Vector2 mPosition;
		// 크기
		Vector2 mScale;
		// 회전 각도
		float mRotation;


	};

}

