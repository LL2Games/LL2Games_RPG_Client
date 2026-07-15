#pragma once
#include "stbResource.h"
#include "stbSingletonBase.h"
#include "stbTexture.h"
namespace stb
{
	class ResourceManager : public SingletonBase<ResourceManager>
	{
	public:

		ResourceManager();
		~ResourceManager();
		
		template <typename T>
		T* Find(const std::wstring& name)
		{
			auto iter = mResources.find(name);

			if (iter == mResources.end())
			{
				return nullptr;
			}

			return dynamic_cast<T*>(iter->second);
		}

		template<typename T>
		T* Load(const std::wstring& name, const std::wstring& path)
		{
			T* resource = Find<T>(name);

			if (resource != nullptr)
			{
				return resource;
			}

			resource = new T();
			if (FAILED(resource->Load(path)))
			{
				assert(false);
			}

			resource->SetName(name);
			resource->SetPath(path);
			mResources.insert(std::make_pair(name, resource));

			return resource;
		}


		
		void Insert(const std::wstring& name, Resource* resource)
		{
			if (name == L"")
				return;
			if (resource == nullptr)
				return;
			mResources.insert(make_pair(name, resource));
		}

		// 1. 월드 / 배경 리소스
		void LoadMapTextures();

		// 2. 게임 오브젝트 리소스
		void LoadPlayerTextures();
		void LoadMonsterTextures();
		void LoadItemTextures();

		// 3. UI 리소스
		void LoadInventoryTextures();
		void LoadQuickSlotTextures();
		void LoadHealthBarTextures();
		void LoadExpTextures();
		void LoadLevelTextures();
		void LoadTradeTextures();

		// 4. 사운드 리소스
		void LoadBGMAudioClips();

		// 5. D2D 렌더링 리소스 초기화
		void LoadAllD2D(stbD2DRenderer& renderer);

	private:
		std::map<std::wstring, Resource*> mResources;
	};

}

