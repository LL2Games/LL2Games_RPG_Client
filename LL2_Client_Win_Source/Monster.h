#pragma once
#include "CommonInclude.h"
#include "MonsterInfo.h"
#include "stbMath.h"
#include "stbGameObject.h"
#include "stbTransform.h"
#include "stbAnimator.h"
#include "BoxCollider2D.h"
#include "stbCircleCollider2D.h"
#include "MonsterScript.h"
#include "CombatSystem_Info.h"

class stbD2DRenderer;

class Monster : public stb::GameObject
{
public:
    void Initialize() override;
    void InitFromSpawn(const MonsterSpawnInfo& info);
    void Update(float deltaTime);
    void Render(stbD2DRenderer& renderer);

    void SetState(MonsterState state);
    void SetPosition(float x, float y);
    void SetAnimation();
    void SetCollider();
    void BindAnimationEvents();
    void OnDamaged(int damage, int curHp, bool dead);
    void OnMove(float x, float y, int dir);

    void ResetFromSpawnInfo(const MonsterSpawnInfo& info);
    void ApplyServerUpdate(const MonsterUpdateInfo& info);
    void ApplyAttackResult(const AttackResult& result);
    void RespawnFromServer(const MonsterUpdateInfo& info);
public:
    int GetInstanceId() const { return m_instanceId; }
    int GetMoveSpeed() const { return m_moveSpeed; }
    bool IsDead() { return m_isDead; }

private:
    int m_instanceId = 0;   // 서버 개체 ID
    int m_monsterId = 0;   // 몬스터 종류 ID

    stb::math::Vector2 m_pos{};
    stb::math::Vector2 m_targetPos{};     // 서버 이동 패킷 받은 위치
    int m_dir = 1;

    int m_curHp = 0;
    int m_maxHp = 0;

    bool m_isDead = false;
    bool m_isDeathAnimationFinished = false;
    MonsterState m_state = MonsterState::E_Idle;

    int m_moveSpeed;

private:
    stb::Transform* m_transform;
    stb::Animator* m_animator;
    stb::Collider* m_collider;

    std::wstring m_currentAnimation;
    MonsterScript* m_script;

    std::string DebugMsg;
};

