#pragma once
#include "CommonInclude.h"
#include "stbGameObject.h"
#include "MonsterInfo.h"
#include "stbTransform.h"
#include "stbAnimator.h"
#include "BoxCollider2D.h"
#include "stbCircleCollider2D.h"


class Projectile : public stb::GameObject
{
public:
    void Initialize() override;
    void InitFromServer(const MonsterProjectileData& info);
    void ApplyServerUpdate(const MonsterProjectileData& info);

    void Update(float deltaTime);
    void Render(stbD2DRenderer& renderer);
    void SetAnimation();

    int GetInstanceId() const { return m_instanceId; }
    bool IsExpired();
private:
    int m_ownerId = 0; //투사체 소유주 Id (현재는 MonsterId)
    int m_instanceId = 0;
    int m_projectileTypeId = 0;

    float m_travelled = 0.0f;
    float m_range = 0.0f;
    float m_speed = 0.0f;

    stb::math::Vector2 m_position{};
    stb::math::Vector2 m_targetPosition{};
    stb::math::Vector2 m_direction{};

    stb::Transform* m_transform = nullptr;
    stb::Animator* m_animator = nullptr;
    stb::Collider* m_collider = nullptr;
};

