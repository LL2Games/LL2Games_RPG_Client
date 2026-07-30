#pragma once
#include "..\\LL2_Client_Win_Source\\stbResourceManager.h"
#include "..\\LL2_Client_Win_Source\\stbTexture.h"
#include "..\\LL2_Client_Win_Source\\stbApplication.h"
#include "..\\LL2_Client_Win_Source\\stbD2DRenderer.h"
#include "stbAudioClip.h"




#define RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()

namespace stb
{
	void LoadResource()
	{
		// ============================
		// 1. 월드 / 배경 리소스
		// ============================

		// 맵 배경
		RESOURCEMANAGER->LoadMapTextures();

		RESOURCEMANAGER->LoadPortalTextures();

		// ============================
		// 2. 게임 오브젝트 리소스
		// ============================

		// 플레이어 이미지 로드
		RESOURCEMANAGER->LoadPlayerTextures();

		// 몬스터 이미지 로드
		RESOURCEMANAGER->LoadMonsterTextures();

		// 아이템 리소스
		RESOURCEMANAGER->LoadItemTextures();


		// ============================
		// 3. UI 리소스
		// ============================

		// 인벤토리
		RESOURCEMANAGER->LoadInventoryTextures();

		// 퀵슬롯 UI 리소스
		RESOURCEMANAGER->LoadQuickSlotTextures();

		// 체력바 UI 리소스
		RESOURCEMANAGER->LoadHealthBarTextures();

		// EXP UI 이미지 로드
		RESOURCEMANAGER->LoadExpTextures();

		// Level UI 이미지 로드
		RESOURCEMANAGER->LoadLevelTextures();

		// 교환창
		RESOURCEMANAGER->LoadTradeTextures();

		// 스탯 UI 이미지 로드
		RESOURCEMANAGER->LoadStatTextures();

		// ============================
		// 4. 사운드 리소스
		// ============================

		// BGM
		RESOURCEMANAGER->LoadBGMAudioClips();

		// ============================
		// 5. D2D 렌더링 리소스 초기화
		// ============================

		// Application이 보유한 Direct2D 렌더러 참조
		stbD2DRenderer& renderer = stb::Application::getInstance()->GetRenderer();

		// 앞에서 로드한 Texture 리소스들을 Direct2D에서 그릴 수 있도록 D2D Bitmap으로 변환/초기화
		RESOURCEMANAGER->LoadAllD2D(renderer);
	}

}
