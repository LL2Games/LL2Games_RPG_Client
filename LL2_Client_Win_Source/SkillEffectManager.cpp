#include "SkillEffectManager.h"

#include "VFXDataManager.h"
#include "VFX_Info.h"

#include "stbGameObject.h"
#include "stbTransform.h"
#include "stbAnimator.h"
#include "stbResourceManager.h"
#include "stbTexture.h"
#include "stbD2DRenderer.h"

#include <algorithm>


#define M_VFXDATAMANAGER stb::SingletonBase<VFXDataManager>::getInstance()
#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()


ActiveSkillEffect::~ActiveSkillEffect()
{
    /*
        현재 GameObject::~GameObject()에서는
        Component를 delete하지 않기 때문에 직접 정리한다.
    */

    delete animator;
    animator = nullptr;

    delete transform;
    transform = nullptr;

    delete object;
    object = nullptr;
}


void SkillEffectManager::Update()
{
    for (auto& effect : m_effects)
    {
        if (effect == nullptr)
            continue;

        if (effect->object == nullptr)
            continue;

        effect->object->Update();
    }

    m_effects.erase(std::remove_if(m_effects.begin(),m_effects.end(),
            [](const std::unique_ptr<ActiveSkillEffect>& effect)
            {
                if (effect == nullptr)
                    return true;

                return effect->finished;
            }
        ),

        m_effects.end()
    );
}


void SkillEffectManager::Render(stbD2DRenderer& renderer)
{
    for (auto& effect : m_effects)
    {
        if (effect == nullptr)
            continue;

        if (effect->object == nullptr)
            continue;

        effect->object->Render(renderer);
    }
}

void SkillEffectManager::PlayCharge(const std::string& vfxKey, const stb::math::Vector2& playerPosition, bool flipX)
{
    const VFXData* vfxData = M_VFXDATAMANAGER->FindVFX(vfxKey);

    if (vfxData == nullptr)
        return;

    if (vfxData->charge.frames.empty())
        return;

    stb::math::Vector2 chargePos = playerPosition;

    //
    // attach_offset.x는
    // 오른쪽 바라보는 상태를 기준으로 작성한다고 가정
    //
    if (flipX)
    {
        chargePos.x += vfxData->charge.attachOffset.x;
    }
    else
    {
        chargePos.x -= vfxData->charge.attachOffset.x;
    }

    chargePos.y += vfxData->charge.attachOffset.y;

    CreateEffect(vfxData->charge, chargePos, flipX);
}

void SkillEffectManager::PlayEffect(const std::string& vfxKey,const stb::math::Vector2& position,bool flipX)
{
    OutputDebugStringA(("[SkillEffectManager] PlayEffect : " + vfxKey + "\n").c_str());

    const VFXData* vfxData = M_VFXDATAMANAGER->FindVFX(vfxKey);

    if (vfxData == nullptr)
    {
        OutputDebugStringA("[SkillEffectManager] VFXData not found\n");
        return;
    }

    if (vfxData->effect.frames.empty())
    {
        OutputDebugStringA("[SkillEffectManager] effect frames empty\n");
        return;
    }

    CreateEffect(vfxData->effect, position, flipX);
}


void SkillEffectManager::PlayHit(const std::string& vfxKey,const stb::math::Vector2& position)
{
    OutputDebugStringA(("[SkillEffectManager] PlayHit : "+ vfxKey + "\n").c_str());

    const VFXData* vfxData = M_VFXDATAMANAGER->FindVFX(vfxKey);

    if (vfxData == nullptr)
    {
        OutputDebugStringA("[SkillEffectManager] VFXData not found\n");
        return;
    }

    if (vfxData->hit.frames.empty())
    {
        OutputDebugStringA("[SkillEffectManager] hit frames empty\n");
        return;
    }

    CreateEffect(vfxData->hit, position, false);
}


bool SkillEffectManager::CreateEffect(const VFXAnimationData& animData, const stb::math::Vector2& position, bool flipX)
{
    if (animData.frames.empty())
        return false;

    // ------------------------------------------------
    // 1. Texture 조회
    // ------------------------------------------------

    std::vector<stb::Texture*> frames;
    frames.reserve(animData.frames.size());

    for (const std::wstring& frameKey : animData.frames)
    {
        stb::Texture* texture = M_RESOURCEMANAGER->Find<stb::Texture>(frameKey);

        if (texture == nullptr)
        {
            OutputDebugStringW(
                (
                    L"[SkillEffectManager] Texture Not Found : "
                    + frameKey
                    + L"\n"
                    ).c_str()
            );

            continue;
        }

        frames.push_back(texture);
    }

    if (frames.empty())
    {
        OutputDebugStringA("[SkillEffectManager] All texture frames are empty\n");
        return false;
    }


    // ------------------------------------------------
    // 2. 이펙트 객체 생성
    // ------------------------------------------------

    auto effect = std::make_unique<ActiveSkillEffect>();
    effect->object = new stb::GameObject();
    effect->transform = effect->object->AddComponent<stb::Transform>();
    effect->animator = effect->object->AddComponent<stb::Animator>();

    if (effect->transform == nullptr || effect->animator == nullptr)
    {
        OutputDebugStringA("[SkillEffectManager] Component create failed\n");
        return false;
    }


    // ------------------------------------------------
    // 3. 플레이어 월드 위치를 그대로 사용
    // ------------------------------------------------

    effect->transform->SetPosition(position);
    effect->object->Initialize();

    // ------------------------------------------------
    // 4. Animation 생성
    // ------------------------------------------------

    effect->animator->CreateFrameAnimation(
        L"SkillVFX",
        frames,
        animData.origin,
        animData.offset,
        animData.frameTime
    );

    // ------------------------------------------------
    // 5. 좌우 반전
    // ------------------------------------------------

    effect->animator->SetFlipX(flipX);


    // ------------------------------------------------
    // 6. 애니메이션 완료 이벤트
    // ------------------------------------------------

    stb::Animator::EventNames eventNames{};

    eventNames.completeEventName = L"SkillVFXComplete";
    effect->animator->SetAnimationEventNames(L"SkillVFX",eventNames);
    ActiveSkillEffect* rawEffect = effect.get();

    effect->animator->RegisterEvent(L"SkillVFXComplete",
        [rawEffect]()
        {
            if (rawEffect == nullptr)
                return;

            rawEffect->finished = true;

            OutputDebugStringA(
                "[SkillEffectManager] Effect Complete\n"
            );
        }
    );


    // ------------------------------------------------
    // 7. 재생
    // ------------------------------------------------

    effect->animator->PlayAnimation( L"SkillVFX",false);

    // ------------------------------------------------
    // 8. Manager 등록
    // ------------------------------------------------

    m_effects.push_back(std::move(effect));
    OutputDebugStringA("[SkillEffectManager] Effect Created\n");
    return true;
}


void SkillEffectManager::Clear()
{
    m_effects.clear();
}
