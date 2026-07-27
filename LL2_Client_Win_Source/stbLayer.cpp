#include "stbLayer.h"
#include "stbLayer.h"
#include "stbD2DRenderer.h"

namespace stb
{
	Layer::Layer()
		: mGameObjects({})
	{

	}

	Layer::~Layer()
	{
	}

	void Layer::Initialize()
	{
		for (const auto& gameObj : mGameObjects)
		{
			if (gameObj == nullptr)
			{
				continue;
			}

			gameObj->Initialize();
		}
	}

	void Layer::Update()
	{
		for (const auto& gameObj : mGameObjects)
		{
			if (gameObj == nullptr)
			{
				continue;
			}

			gameObj->Update();
		}
	}

	void Layer::LateUpdate()
	{
		for (const auto& gameObj : mGameObjects)
		{
			if (gameObj == nullptr)
			{
				continue;
			}

			gameObj->LateUpdate();
		}
	}

	void Layer::Render(HDC hdc)
	{
		for (const auto& gameObj : mGameObjects)
		{
			if (gameObj == nullptr)
			{
				continue;
			}

			gameObj->Render(hdc);
		}
	}

	void Layer::Render(stbD2DRenderer& renderer) 
	{
		for (const auto& gameObj : mGameObjects)
		{
			if (gameObj == nullptr)
			{
				continue;
			}

			gameObj->Render(renderer);
		}
	}

	void Layer::AddGameObject(GameObject* gameObject)
	{
		if (gameObject == nullptr)
		{
			return;
		}

		mGameObjects.push_back(gameObject);
	}

	bool Layer::RemoveGameObject(GameObject* gameObject)
	{
		auto iter = std::find(
			mGameObjects.begin(),
			mGameObjects.end(),
			gameObject
		);

		if (iter == mGameObjects.end())
			return false;

		mGameObjects.erase(iter);
		return true;
	}
}
