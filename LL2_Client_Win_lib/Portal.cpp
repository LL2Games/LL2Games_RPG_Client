#include "Portal.h"
#include "stbD2DRenderer.h"
#include "stbRender.h"
#include "stbInput.h"
#include "stbTransform.h"
#include "stbSceneManager.h"
#include "PlayerManager.h"
#include "stbPlayer.h"
#include "stbNetworkManager.h"
#include "PortalPacketHandler.h"
#include "UIManager.h"
#include "stbApplication.h"
#include <cmath>

using namespace stb;

#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()
#define M_SCENEMANAGER stb::SingletonBase<stb::SceneManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()

void Portal::Initialize()
{
    m_collider = AddComponent<BoxCollider2D>();

    // 포탈 중심으로부터 반너비, 반높이
    m_collider->SetSize(m_triggerHalfSize);
    m_collider->SetOffset(math::Vector2::Zero);
    GameObject::Initialize();
}

void Portal::Update()
{
    GameObject::Update();
    if (m_transitioning || m_destinationScene.empty()) return;
    auto* player = M_PLAYERMANAGER->GetLocalPlayer();
    if (!player || player->IsDead() || !player->GetMovementScript()->CanUsePortal() ||
        UIManager::getInstance()->IsInputFocused() || UIManager::getInstance()->IsGameplayInputBlocked() ||
        GetForegroundWindow() != GetAncestor(stb::Application::getInstance()->GetHWND(), GA_ROOT)) return;
    auto* tr = GetComponent<Transform>();
    auto* playerTransform = player->GetComponent<Transform>();
    if (!tr || !playerTransform || !M_INPUT->GetActionDown(eActionCode::MoveUp)) return;
    auto delta = playerTransform->GetPosition() - tr->GetPosition();
    if (delta.x * delta.x + delta.y * delta.y > m_interactionRange * m_interactionRange) return;
    m_transitioning = true;
    PortalPacketHandler::SendPortalEnter(m_portalId);
}

void Portal::Render(stbD2DRenderer& renderer)
{
    GameObject::Render(renderer);

    if (m_texture == nullptr)
        return;

    ID2D1Bitmap* bitmap = m_texture->GetD2DBitmap();

    if (bitmap == nullptr)
        return;

    Transform* transform = GetComponent<Transform>();

    if (transform == nullptr)
        return;

    math::Vector2 position = transform->GetPosition();

    if (render::mainCamera != nullptr)
    {
        position = render::mainCamera->CalculatePosition(position);
    }

    renderer.DrawBitmap(
        bitmap,
        position.x - m_renderSize.x * 0.5f,
        position.y - m_renderSize.y * 0.5f,
        m_renderSize.x,
        m_renderSize.y
    );
}

void Portal::SetDestination(const std::wstring& sceneName, const math::Vector2& spawnPosition)
{
    m_destinationScene = sceneName;
    m_spawnPosition = spawnPosition;
}

void Portal::SetTriggerHalfSize(const stb::math::Vector2& halfSize)
{
    stb::math::Vector2 portalColliderSize = { halfSize.x * 0.5f,  halfSize.y * 0.5f };

    m_triggerHalfSize = portalColliderSize;

    if (m_collider != nullptr)
    {
        m_collider->SetSize(m_triggerHalfSize);
    }
}

void Portal::SetTexture(stb::Texture* texture)
{
    m_texture = texture;
}


void Portal::SetRenderSize(const stb::math::Vector2& size)
{
    m_renderSize = size;
}
