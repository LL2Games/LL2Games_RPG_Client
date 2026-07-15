#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "stbScene.h"

class stbD2DRenderer;

namespace stb
{
	class SceneManager : public SingletonBase<SceneManager>
	{
	public:
		SceneManager();
		~SceneManager();

		template <typename T>
		Scene* CreateScene(const std::wstring& name)
		{
			T* scene = new T();
			scene->SetName(name);
			mActiveScene = scene;
			scene->Initialize();
			mScenes.insert(std::make_pair(name, scene));
			return scene;
		}

		void Initialize();
		void Update();
		void Render(HDC hdc);
		void Render(stbD2DRenderer& renderer);

		Scene* LoadScene(const std::wstring& name);
		Scene* GetActiveScene() { return mActiveScene; }
		void SetActiveScene(Scene* scene) { mActiveScene = scene; }
		Scene* GetDontDestroyOnLoad() { return mDontDestroyOnLoad; }

		std::vector<GameObject*> GetGameObjects(eLayerType layer);

		void SetOverlayScene(const std::wstring& name);
		void ClearOverlayScene() { mOverlayScene = nullptr; }

	private:
		std::map<std::wstring, Scene*> mScenes;
		Scene* mActiveScene;
		Scene* mDontDestroyOnLoad;
		Scene* mOverlayScene = nullptr;
	};
}
