#pragma once
#include "stbEntity.h"

class stbD2DRenderer;

namespace stb
{
	using namespace enums;
	
	class GameObject;

	class Component : public Entity
	{
	public:
		Component(eComponentType type);
		~Component();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render(HDC hdc);
		virtual void Render(stbD2DRenderer& renderer);

		virtual void SetOwner(GameObject* gameObject) { mGameObject = gameObject; }
		GameObject* GetOwner() { return mGameObject; }
		eComponentType GetType() { return mType;}

	private:
		GameObject* mGameObject;
		enums::eComponentType mType;


	};

}


