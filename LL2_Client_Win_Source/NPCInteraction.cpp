#include "NPCInteraction.h"

#include "stbGameObject.h"
#include "stbTransform.h"
#include "stbCamera.h"
#include "stbRender.h"
#include "PlayerManager.h"

#include <cmath>
#include <utility>

namespace stb
{
    void NPCInteraction::Setup(int spawnId, int npcId, float interactionRange)
    {
        m_spawnId = spawnId;
        m_npcId = npcId;

        m_interactionRange = std::isfinite(interactionRange) && interactionRange > 0.0f ? interactionRange: 100.0f;

        m_requestPending = false;
    }

    void NPCInteraction::SetRequestCallback(RequestCallback callback)
    {
        m_onRequest = std::move(callback);
    }

    void NPCInteraction::SetRenderInfo(math::Vector2 size,math::Vector2 origin)
    {
        m_renderSize = size;
        m_origin = origin;
    }

    bool NPCInteraction::HitTest(float mouseX, float mouseY)
    {
        GameObject* owner = GetOwner();
        if (owner == nullptr)
            return false;

        Transform* transform = owner->GetComponent<Transform>();
        if (transform == nullptr)
            return false;

        math::Vector2 position = transform->GetPosition();

        // NPC 월드 좌표를 화면 좌표로 변환
        if (render::mainCamera != nullptr)
        {
            position = render::mainCamera->CalculatePosition(position);
        }

        const float left = position.x - m_origin.x;
        const float top = position.y - m_origin.y;

        return mouseX >= left && mouseX < left + m_renderSize.x  && mouseY >= top && mouseY < top + m_renderSize.y;
    }

    bool NPCInteraction::IsInRange()
    {
        GameObject* owner = GetOwner();

        if (owner == nullptr)
            return false;

        Player* player = SingletonBase<PlayerManager>::getInstance()->GetLocalPlayer();

        if (player == nullptr || player->IsDead())
            return false;

        Transform* npcTransform = owner->GetComponent<Transform>();

        Transform* playerTransform = player->GetComponent<Transform>();

        if (npcTransform == nullptr || playerTransform == nullptr)
            return false;

        const math::Vector2 npcPosition = npcTransform->GetPosition();

        const math::Vector2 playerPosition = playerTransform->GetPosition();

        const float dx = playerPosition.x - npcPosition.x;
        const float dy = playerPosition.y - npcPosition.y;

        // 제곱 거리로 비교하므로 sqrt 불필요
        return dx * dx + dy * dy <= m_interactionRange * m_interactionRange;
    }

    bool NPCInteraction::TryInteract()
    {
        if (m_requestPending || !m_onRequest)
            return false;

        if (!IsInRange())
            return false;

        m_requestPending = true;

        if (!m_onRequest(m_spawnId))
        {
            m_requestPending = false;
            return false;
        }

        return true;
    }
}
