#pragma once
#include "CommonInclude.h"
#include "stbComponent.h"

class stbD2DRenderer;

namespace stb
{
	class GameObject : public Entity
	{
	public:
		GameObject();
		~GameObject();

		template <typename T>
		T* AddComponent()
		{
			T* component = new T();
			component->SetOwner(this);
			mComponents[(UINT)component->GetType()] = component;
			return component;
		}

		template<typename T>
		T* GetComponent()
		{
			T* component = nullptr;
			for (Component* comp : mComponents)
			{
				component = dynamic_cast<T*>(comp);
				if (component)
					break;
			}
			return component;
		}

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render(HDC hdc);
		virtual void Render(stbD2DRenderer& renderer);

		void SetLayerType(eLayerType type) { mLayerType = type; }
		eLayerType GetLayerType() { return mLayerType; }

	private:
		std::vector<Component*> mComponents;
		eLayerType mLayerType;
	};
}
