#pragma once
#include "..\\LL2_Client_Win_Source\\stbGameObject.h"
#include "..\\LL2_Client_Win_Source\\BoxCollider2D.h"
#include "..\\LL2_Client_Win_Source\\stbTexture.h"

class Portal : public stb::GameObject
{
public:
    void Initialize() override;
    void Update() override;
    void Render(stbD2DRenderer& renderer) override;

    void SetDestination(
        const std::wstring& sceneName,
        const stb::math::Vector2& spawnPosition
    );

    void SetTriggerHalfSize(const stb::math::Vector2& halfSize);
    void SetTexture(stb::Texture* texture);
    void SetRenderSize(const stb::math::Vector2& size);
    void SetPortalId(const std::string& portalId) { m_portalId = portalId; }
    void ResetTransition() { m_transitioning = false; }
private:
    std::string m_portalId;
    std::wstring m_destinationScene;
    stb::math::Vector2 m_spawnPosition;
    stb::BoxCollider2D* m_collider = nullptr;
    bool m_transitioning = false;

    stb::math::Vector2 m_triggerHalfSize = stb::math::Vector2(60.0f, 100.0f);

    stb::Texture* m_texture = nullptr;

    stb::math::Vector2 m_renderSize = stb::math::Vector2(100.0f, 160.0f);
};

