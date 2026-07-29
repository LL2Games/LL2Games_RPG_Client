#pragma once

#include "..\\LL2_Client_Win_Source\\stbSceneManager.h"

#include "GameSession.h"
#include "stbPlayScene.h"
#include "stbChatScene.h"
#include "Map_100000000.h"
#include "Map_100000001.h"
#include "Map_100000002.h"

#define SCENEMANAGER stb::SingletonBase<stb::SceneManager>::getInstance()
#define M_GAMESESSION stb::SingletonBase<GameSession>::getInstance()

namespace stb
{
	void LoadScene()
	{
		M_GAMESESSION->InitializeOnce();

		//SCENEMANAGER->CreateScene<PlayScene>(L"PlayScene");
		SCENEMANAGER->CreateScene<ChatScene>(L"ChatScene");
		SCENEMANAGER->CreateScene<Map_100000000>(L"Map_100000000");
		SCENEMANAGER->CreateScene<Map_100000001>(L"Map_100000001");
		SCENEMANAGER->CreateScene<Map_100000002>(L"Map_100000002");

		SCENEMANAGER->LoadScene(L"Map_100000000");
		SCENEMANAGER->SetOverlayScene(L"ChatScene");
	}

}

