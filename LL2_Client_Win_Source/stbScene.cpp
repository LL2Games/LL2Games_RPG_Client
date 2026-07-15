#include "stbScene.h"
#include "stbD2DRenderer.h"

namespace stb
{
	Scene::Scene()
		: mLayers({})
	{
		CreateLayers();
	}

	Scene::~Scene()
	{
	}

	void Scene::Initialize()
	{
		for (const auto& layer : mLayers)
		{
			if (layer == nullptr)
			{
				continue;
			}

			layer->Initialize();
		}
	}

	void Scene::Update()
	{
		for (const auto& layer : mLayers)
		{
			if (layer == nullptr)
			{
				continue;
			}

			layer->Update();
		}
	}

	void Scene::LateUpdate()
	{
		for (const auto& layer : mLayers)
		{
			if (layer == nullptr)
			{
				continue;
			}

			layer->LateUpdate();
		}
	}

	void Scene::Render(HDC hdc)
	{
		for (const auto& layer : mLayers)
		{
			if (layer == nullptr)
			{
				continue;
			}

			layer->Render(hdc);
		}
	}

	void Scene::Render(stbD2DRenderer& renderer)
	{
		for (const auto& layer : mLayers)
		{
			if (layer == nullptr)
			{
				continue;
			}

			layer->Render(renderer);
		}

		dropItemManager.Render(renderer);
	}


	void Scene::OnEnter()
	{

	}

	void Scene::OnExit()
	{

	}

	void Scene::CreateLayers()
	{
		mLayers.resize((UINT)eLayerType::Max);
		for (size_t i = 0; i < (UINT)eLayerType::Max; i++)
		{
			mLayers[i] = new Layer();
		}
	}

	void Scene::AddGameObject(GameObject* gameObject, const eLayerType layerType)
	{
		Layer* layer = mLayers[(UINT)layerType];
		if (layer == nullptr)
		{
			return;
		}
		layer->AddGameObject(gameObject);

	}
}
