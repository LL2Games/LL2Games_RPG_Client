#include "stbChatScene.h"

#include "stbNetworkDebug.h"

#include "stbApplication.h"
#include "stbChatNetworkManager.h"
#include "ChatPacketHandler.h"
#include "stbInput.h"

#include "StringConvert.h"

//#include "PacketManager.h"
//#include "stbLogger.h"
//#include "PlayerManager.h"
//#include "stbTime.h"
//#include "stbGameObject.h"
//#include "stbPlayer.h"
//#include "stbObject.h"
//#include "stbTransform.h"
//#include "stbTexture.h"
//#include "stbResourceManager.h"
//#include "stbSpriteRenderer.h"
//#include "stbPlayerScript.h"
//#include "stbCamera.h"
//#include "stbAnimator.h"
//#include "stbRender.h"
#include "UIManager.h"

#define M_CHAT_NETMANAGER stb::SingletonBase<ChatNetworkManager>::getInstance()
#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()
#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()

//#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
//#define M_PKMANAGER stb::SingletonBase<PacketManager>::getInstance()
//#define M_PLMANAGER stb::SingletonBase<PlayerManager>::getInstance()
//#define M_TIME	stb::SingletonBase<stb::Time>::getInstance()

namespace stb
{
	ChatScene::ChatScene() // :mPlayer(nullptr)
	{
	}

	ChatScene::~ChatScene()
	{
	}

	void ChatScene::Initialize()
	{
		Scene::Initialize();

		// 채팅용 네트워크 매니저 초기화
		HWND hWnd = stb::Application::getInstance()->GetHWND();

		// 핸들러 등록
		ChatNetworkManager::getInstance()->RegisterHandler(PKT_CHAT_INIT,
			[](const ParsedPacket& pkt) { ChatPacketHandler::HandleChatInit(pkt); });
		ChatNetworkManager::getInstance()->RegisterHandler(PKT_CHAT,
			[](const ParsedPacket& pkt) { ChatPacketHandler::HandleChat(pkt); });

		// 채팅 서버 접속 (PlayScene의 채널 서버와는 별개)
		ChatNetworkManager::getInstance()->Connect(
			g_ServerIp,
			//NetworkConfig::SERVER_IP,
			g_ChatPort,
			hWnd);

	}

	void ChatScene::Update()
	{
		Scene::Update();
		M_UIMANAGER->Update();
		// Enter 키 → 입력 모드 토글 OR 전송
		if (M_INPUT->GetKeyDown(eKeyCode::Enter))
		{
#if 1
			if (M_UIMANAGER->ConsumeTradeQuantityEnter())
				return;

			if (M_UIMANAGER->IsTradeRequestActive())
				return;

			if (M_UIMANAGER->IsInputFocused())
			{
				M_UIMANAGER->SubmitChatInput();
			}
			else
			{
				M_UIMANAGER->ToggleChatInput();
			}
#else
			if (M_UIMANAGER->IsInputFocused())
			{
				// 입력 모드 → Enter = 전송 후 닫기
				// SubmitInput은 ChatUI 내부에서 처리하므로 UIManager 경유
				M_UIMANAGER->SubmitChatInput();
			}
			else
			{
				// 비활성 → Enter = 채팅창 열기
				M_UIMANAGER->ToggleChatInput();
			}
#endif
		}

	}

	void ChatScene::LateUpdate()
	{
		Scene::LateUpdate();
	}

	void ChatScene::Render(HDC hdc)
	{
		Scene::Render(hdc);
	}

	void ChatScene::Render(stbD2DRenderer& renderer)
	{
		Scene::Render(renderer);
		M_UIMANAGER->Render(renderer);

		// 화면 하단에 반투명 박스 + 텍스트 라인들 + 입력박스 그리기
		
		
		//DrawChatBackdrop(renderer);
		//DrawChatLog(renderer, mLogLines);
		//if (mInputActive) DrawChatInput(renderer, mInputBuffer);
		
	}

	void ChatScene::OnExit()
	{
		Scene::OnExit();
	}

	void ChatScene::OnEnter()
	{
		Scene::OnEnter();
	}

	void ChatScene::ToggleInputFocus()
	{
	}

}
