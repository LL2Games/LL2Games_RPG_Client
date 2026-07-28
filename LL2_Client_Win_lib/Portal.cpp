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

    if (m_transitioning || m_destinationScene.empty())
        return;

    Player* player = M_PLAYERMANAGER->GetLocalPlayer();

    if (player == nullptr || m_collider == nullptr)
        return;

    Transform* portalTransform = GetComponent<Transform>();
    Transform* playerTransform = player->GetComponent<Transform>();
    BoxCollider2D* playerCollider = player->GetComponent<BoxCollider2D>();

    if (portalTransform == nullptr || playerTransform == nullptr || playerCollider == nullptr)
    {
        return;
    }

    math::Vector2 portalCenter = portalTransform->GetPosition() + m_collider->GetOffset();

    math::Vector2 playerCenter = playerTransform->GetPosition() + playerCollider->GetOffset();

    math::Vector2 portalHalfSize = m_collider->GetSize();

    math::Vector2 playerHalfSize = playerCollider->GetSize();

    bool isOverlapping = std::abs(playerCenter.x - portalCenter.x) <= playerHalfSize.x + portalHalfSize.x &&
        std::abs(playerCenter.y - portalCenter.y) <= playerHalfSize.y + portalHalfSize.y;

    if (!isOverlapping)
        return;

    if (!M_INPUT->GetActionDown(eActionCode::MoveUp))
        return;

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
    m_triggerHalfSize = halfSize;

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
