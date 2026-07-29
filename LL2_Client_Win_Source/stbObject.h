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

	// 지정한 GameObject를 씬 전환 후에도 유지되는 전용 씬으로 이동시킨다.
	// TODO: 기존 Scene에서 객체 제거 및 중복 등록 방지 처리 추가
	// maybe_unused: 현재 사용 안할때 넣는 키워드
	[[maybe_unused]]
	inline void SetDontDestroyOnLoad(GameObject* gameObject)
	{
		if (gameObject == nullptr)
			return;

		Scene* activeScene =
			M_SCENEMANAGER->GetActiveScene();

		Scene* dontDestroyOnLoad =
			M_SCENEMANAGER->GetDontDestroyOnLoad();

		if (activeScene == nullptr ||
			dontDestroyOnLoad == nullptr ||
			activeScene == dontDestroyOnLoad)
		{
			return;
		}

		enums::eLayerType layerType =
			gameObject->GetLayerType();

		Layer* activeLayer =
			activeScene->GetLayer(layerType);

		// 기존 맵에서 제거
		if (!activeLayer->RemoveGameObject(gameObject))
			return;
		

		/*Scene* activeScene = M_SCENEMANAGER->GetActiveScene();

		Scene* dontDestroyOnLoad = M_SCENEMANAGER->GetDontDestroyOnLoad();
		dontDestroyOnLoad->AddGameObject(gameObject, gameObject->GetLayerType());*/
		// DontDestroyOnLoad로 이동
		dontDestroyOnLoad->AddGameObject(
			gameObject,
			layerType
		);

	}


}

