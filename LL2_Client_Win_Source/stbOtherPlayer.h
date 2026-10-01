#pragma once
#include "CommonInclude.h"
#include "stbGameObject.h"
#include "playerInfo.h"
#include "stbTransform.h"
#include "stbAnimator.h"
#include "stbDamageText.h"
#include "BoxCollider2D.h"
#include "EquipeTypes.h"
#include "MovementStream.h"
#include "MovementVisual.h"

class stbD2DRenderer;

namespace stb
{
    class OtherPlayer : public GameObject
    {
    public:
        OtherPlayer();
        virtual ~OtherPlayer();

        void Initialize() override;
        void Update() override;
        void LateUpdate() override;
        void Render(HDC hdc) override;
        void Render(stbD2DRenderer& renderer) override;

        void SetCharacterId(const std::string& id) { mCharacterId = id; }
        std::string GetCharacterId() const { return mCharacterId; }

        void SetNickName(const std::string& nickname) { m_nickName = nickname; }
        std::string GetNickName() const { return m_nickName; }

        void UpdatePosition(float x, float y);
        void SetTargetPosition(float x, float y, float speed);
        void SetDirection(int dir);
        void SetState(PlayerState state);
        void ApplyMovementSnapshot(const movement::Snapshot& snapshot);
        bool HasMovementSnapshot() const { return m_movement.HasSnapshot(); }
        void ResetMovementConnection() { m_movement.Reset(); m_movementVisual.Reset(); }

        PlayerState GetState() { return m_playerState; }
        void AddFollower(GameObject* obj, Vector2 offset)
        {
            mFollowers.push_back({ obj, offset });
        }

        int GetWeaponTypeToInt() { return static_cast<int>(m_weaponType); }
        WeaponType GetWeaponType() { return m_weaponType; }

    private:
        void SyncFollowers(Vector2 pos);

        std::string mCharacterId;
        std::string m_nickName;
        Vector2 mTargetPosition;
        float mTargetSpeed;
        bool mHasTarget;
        float mInterpolationSpeed;
        movement::Stream m_movement;
        movement::Visual m_movementVisual;

        // 플레이어 상태
        PlayerState m_playerState;

        struct Follower { GameObject* obj; Vector2 offset; };
        std::vector<Follower> mFollowers;

    private:
        stb::Transform* m_transform;
        stb::Animator* m_animator;
        stb::DamageText* m_damageText;
        stb::BoxCollider2D* m_collider;

        std::wstring m_currentAnimation;

        WeaponType m_weaponType = WeaponType::None;
    };
}
