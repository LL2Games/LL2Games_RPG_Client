#include "MovementDebug.h"
#include "stbOtherPlayer.h"
#include "stbTransform.h"
#include "stbAnimator.h"
#include "stbResourceManager.h"
#include "stbTexture.h"
#include "stbTime.h"
#include "MapDataManager.h"
#include "stbD2DRenderer.h"
#include "stbRender.h"
#include "stbCamera.h"
#include "StringConvert.h"

#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_Time stb::SingletonBase<stb::Time>::getInstance()

namespace stb
{
    OtherPlayer::OtherPlayer()
        : mCharacterId("")
        , mTargetPosition(Vector2::Zero)
        , mTargetSpeed(0.0f)
        , mHasTarget(false)
        , mInterpolationSpeed(800.0f) // 500 -> 800으로 증가 (더 빠르게 따라감)
        , m_playerState(PlayerState::None)
        , m_transform(nullptr)
        , m_animator(nullptr)
        , m_damageText(nullptr)
        , m_collider(nullptr)
    {
    }

    OtherPlayer::~OtherPlayer()
    {
    }

    void OtherPlayer::Initialize()
    {
        GameObject::Initialize();
        m_transform = AddComponent<stb::Transform>();
        m_animator = AddComponent<stb::Animator>();
        m_damageText = AddComponent<stb::DamageText>();
        m_collider = AddComponent<stb::BoxCollider2D>();
    }

    void OtherPlayer::Update()
    {
        GameObject::Update();
        movement::Snapshot displayed;
        if (!m_movement.Update(M_Time->GetDeltaTime(), displayed)) return;
        m_transform->SetPosition({displayed.position.x, displayed.position.y});
        SyncFollowers({displayed.position.x, displayed.position.y});
        const auto* map = MapDataManager::getInstance()->FindMapData(displayed.mapId);
        const auto* climb = map ? map->physics.FindClimbable(displayed.climbableId) : nullptr;
        const bool action = m_playerState >= PlayerState::Attack && m_playerState < PlayerState::Skill_End;
        m_movementVisual.Update(m_animator, displayed, M_Time->GetDeltaTime(), climb && climb->ladder,
            action && displayed.mode != movement::Mode::Climbing && !movement::IsStunned(displayed));
    }

    void OtherPlayer::ApplyMovementSnapshot(const movement::Snapshot& s)
    {
        const bool reset = !m_movement.HasSnapshot() || s.epoch != m_movement.Latest().epoch;
        const int previousLife = m_movement.HasSnapshot() ? m_movement.Latest().lifeState : -1;
        if (!m_movement.Push(s)) return;
        if (reset)
        {
            m_movementVisual.Reset();
            m_transform->SetPosition({s.position.x, s.position.y});
            SyncFollowers({s.position.x, s.position.y});
            SetState(movement::IsDead(s) ? PlayerState::Dead : PlayerState::Idle);
        }
        if (movement::IsDead(s)) SetState(PlayerState::Dead);
        else if (s.mode == movement::Mode::Climbing || movement::IsStunned(s)) SetState(PlayerState::Idle);
        else if (s.lifeState == static_cast<int>(movement::PlayerLife::Attack) && previousLife != s.lifeState)
            SetState(PlayerState::Attack);
        mHasTarget = false;
    }

    void OtherPlayer::LateUpdate()
    {
        GameObject::LateUpdate();
    }

    void OtherPlayer::Render(HDC hdc)
    {
        GameObject::Render(hdc);
    }

    void OtherPlayer::Render(stbD2DRenderer& renderer)
    {
        GameObject::Render(renderer);
        if (m_transform) movement::DrawOriginAndFeet(renderer, m_transform->GetPosition(), movement::PlayerFootOffset);

        if (m_transform == nullptr ||
            m_nickName.empty())
        {
            return;
        }

        Vector2 screenPos = m_transform->GetPosition();

        if (render::mainCamera != nullptr)
        {
            screenPos =
                render::mainCamera->CalculatePosition(screenPos);
        }

        D2D1_RECT_F nameRect = D2D1::RectF(
            screenPos.x - 60.0f,
            screenPos.y - 85.0f,
            screenPos.x + 60.0f,
            screenPos.y - 60.0f
        );

        std::wstring name =
            Convert::StringToWString(m_nickName);

        // 그림자
        D2D1_RECT_F shadowRect = nameRect;
        shadowRect.left += 1.0f;
        shadowRect.right += 1.0f;
        shadowRect.top += 1.0f;
        shadowRect.bottom += 1.0f;

        renderer.DrawTextString(
            name,
            shadowRect,
            D2D1::ColorF(D2D1::ColorF::Black),
            TextStyle::NickName
        );

        // 본문
        renderer.DrawTextString(
            name,
            nameRect,
            D2D1::ColorF(D2D1::ColorF::White),
            TextStyle::NickName
        );
    }


    void OtherPlayer::UpdatePosition(float x, float y)
    {
        if (m_movement.HasSnapshot()) return;
        Transform* tr = GetComponent<Transform>();
        if (tr)
        {
            tr->SetPosition(Vector2(x, y));
        }
    }

    void OtherPlayer::SetTargetPosition(float x, float y, float speed)
    {
        if (m_movement.HasSnapshot()) return;
        mTargetPosition = Vector2(x, y);
        mTargetSpeed = speed;
        mHasTarget = true;
    }

    void OtherPlayer::SetDirection(int dir)
    {
        stb::Animator* animator = GetComponent<stb::Animator>();
        if (animator == nullptr)
            return;

        animator->SetFlipX(dir > 0);
    }

    void OtherPlayer::SetState(PlayerState state)
    {
        if (m_playerState == state)
            return;

        m_playerState = state;
        if (m_animator) m_animator->SetPaused(false);

        switch (state)
        {
        case PlayerState::Idle:
            m_currentAnimation = L"stand";
            break;

        case PlayerState::Walk:
            m_currentAnimation = L"walk";
            break;

        case PlayerState::Dead:
            m_currentAnimation = L"dead";
            break;
        case PlayerState::Skill_Slash:
            m_currentAnimation = L"slash";
            break;
        case PlayerState::Attack:
            m_currentAnimation = L"swingO3";
            break;

        default:
            m_currentAnimation = L"stand";
            break;
        }

        stb::Animator* animator = GetComponent<stb::Animator>();
        if (animator != nullptr)
        {
            bool isLoop = true;

            if ((state >= PlayerState::Attack && state < PlayerState::Skill_End) || state == PlayerState::Dead)
                isLoop = false;

            animator->PlayAnimation(m_currentAnimation, isLoop);
        }
    }

    void OtherPlayer::SyncFollowers(Vector2 pos)
    {
        for (auto& f : mFollowers)
        {
            if (f.obj)
            {
                Transform* tr = f.obj->GetComponent<Transform>();
                if (tr) tr->SetPosition(Vector2(pos.x + f.offset.x, pos.y + f.offset.y));
            }
        }
    }
}
