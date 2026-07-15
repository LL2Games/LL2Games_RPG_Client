#include "stbOtherPlayer.h"
#include "stbTransform.h"
#include "stbAnimator.h"
#include "stbResourceManager.h"
#include "stbTexture.h"
#include "stbTime.h"
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
        
        // 목표 위치가 있으면 부드럽게 이동
        if (mHasTarget)
        {
            Transform* tr = GetComponent<Transform>();
            if (tr)
            {
                Vector2 currentPos = tr->GetPosition();
                Vector2 direction = mTargetPosition - currentPos;
                float distance = direction.length();
                
                if (distance > 0.5f) // 목표에 거의 도달하지 않았으면
                {
                    // Lerp 방식: 거리에 비례해서 부드럽게 이동
                    float lerpFactor = 10.0f * M_Time->GetDeltaTime(); // 초당 10배 속도로 따라감
                    if (lerpFactor > 1.0f) lerpFactor = 1.0f;
                    
                    Vector2 newPos = currentPos + direction * lerpFactor;
                    tr->SetPosition(newPos);
                    SyncFollowers(newPos);
                }
                else
                {
                    // 목표에 도달
                    tr->SetPosition(mTargetPosition);
                    SyncFollowers(mTargetPosition);
                    mHasTarget = false;
                    if (m_playerState == PlayerState::Walk)
                    {
                        SetState(PlayerState::Idle);
                    }
                }
            }
        }
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
        Transform* tr = GetComponent<Transform>();
        if (tr)
        {
            tr->SetPosition(Vector2(x, y));
        }
    }

    void OtherPlayer::SetTargetPosition(float x, float y, float speed)
    {
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

        switch (state)
        {
        case PlayerState::Idle:
            m_currentAnimation = L"stand";
            break;

        case PlayerState::Walk:
            m_currentAnimation = L"walk";
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

            if (state == PlayerState::Attack)
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
