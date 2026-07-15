#pragma once
#include "stbComponent.h"
#include "stbLayer.h"
#include "stbScene.h"
#include "stbSceneManager.h"
#include "stbTransform.h"

#define M_SCENEMANAGER stb::SingletonBase<stb::SceneManager>::getInstance()

namespace stb::object
{
	template<typename T>
	static T* Instantiate(stb::enums::eLayerType type)
	{
		T* gameObject = new T();
		gameObject->SetLayerType(type);
		Scene* activeScene = M_SCENEMANAGER->GetActiveScene();
		Layer* layer = activeScene->GetLayer(type);
		layer->AddGameObject(gameObject);

		return gameObject;
	}

	template<typename T>
	static T* Instantiate(stb::enums::eLayerType type, math::Vector2 position)
	{
		T* gameObject = new T();
		gameObject->SetLayerType(type);
		Scene* activeScene = M_SCENEMANAGER->GetActiveScene();
		Layer* layer = activeScene->GetLayer(type);
		layer->AddGameObject(gameObject);

		gameObject->AddComponent<Transform>();
		Transform* tr = gameObject->GetComponent<Transform>();
		tr->SetPosition(position);


		return gameObject;
	}

	static void SetDontDestroyOnLoad(GameObject* gameObject)
	{
		Scene* activeScene = M_SCENEMANAGER->GetActiveScene();
		
		Scene* dontDestroyOnLoad = M_SCENEMANAGER->GetDontDestroyOnLoad();
		dontDestroyOnLoad->AddGameObject(gameObject, gameObject->GetLayerType());
	}


}