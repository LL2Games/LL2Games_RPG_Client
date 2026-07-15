#include "stbGameObject.h"
#include "stbApplication.h"
#include "stbD2DRenderer.h"

namespace stb
{
	GameObject::GameObject()
		:mLayerType(eLayerType::None)
	{
		mComponents.resize((UINT)eComponentType::EnumsEnd);
	}

	GameObject::~GameObject()
	{

	}

	void GameObject::Initialize()
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
			{
				continue;
			}

			comp->Initialize();
		}
	}

	void GameObject::Update()
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
			{
				continue;
			}

			comp->Update();
		}
	}

	void GameObject::LateUpdate()
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
			{
				continue;
			}

			comp->LateUpdate();
		}
	}

	void GameObject::Render(HDC hdc)
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
			{
				continue;
			}

			comp->Render(hdc);
		}
	}

	void GameObject::Render(stbD2DRenderer& renderer)
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
			{
				continue;
			}

			comp->Render(renderer);
		}
	}

}
