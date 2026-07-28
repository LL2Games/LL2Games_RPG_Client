#include "GameSession.h"
#include "stbObject.h"
#include "stbRender.h"
#include "stbApplication.h"
#include "stbNetworkDebug.h"
#include "UIManager.h"
#include "MonsterManager.h"
#include "PacketManager.h"
#include "stbLogger.h"
#include "PlayerManager.h"
#include "stbPlayer.h"

#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()
#define M_MONSTERMANAGER stb::SingletonBase<MonsterManager>::getInstance()
#define M_PKMANAGER stb::SingletonBase<PacketManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()

using namespace stb;


void GameSession::InitializeOnce()
{
    if (m_initialized)
        return;

    stb::Logger::Init();
    M_UIMANAGER->Init();
    M_MONSTERMANAGER->Init();
    M_PKMANAGER->RegisterAllHandlers();

    HWND hWnd = stb::Application::getInstance()->GetHWND();
    stb::InitializeNetworkDebug(hWnd);

    m_initialized = true;
}

void GameSession::EnsurePersistentObjects()
{
    Player* player = M_PLAYERMANAGER->GetLocalPlayer();

    if (player == nullptr)
    {
        player = M_PLAYERMANAGER->CreateLocalPlayer(enums::eLayerType::Player,math::Vector2(300.0f, 700.0f));
        object::SetDontDestroyOnLoad(player);
    }

    if (render::mainCamera == nullptr)
    {
        GameObject* cameraObject = object::Instantiate<GameObject>( enums::eLayerType::Particle, Vector2::Zero);
        Camera* camera = cameraObject->AddComponent<Camera>();

        camera->SetTarget(player);
        cameraObject->Initialize();
        render::mainCamera = camera;
        object::SetDontDestroyOnLoad(cameraObject);
    }
}
