#include "NPC.h"


#include "NPCDataManager.h"
#include "stbTransform.h"
#include "stbAnimator.h"
#include "stbCamera.h"
#include "stbRender.h"
#include "stbResourceManager.h"
#include "StringConvert.h"

#include <cmath>
#include <string>
#include <vector>

namespace stb
{
    void NPC::Initialize()
    {
        // 기존 컴포넌트가 있으면 재사용
        m_transform = GetComponent<Transform>();

        if (m_transform == nullptr)
            m_transform = AddComponent<Transform>();

        m_animator = GetComponent<Animator>();
        if (m_animator == nullptr)
            m_animator = AddComponent<Animator>();

        // 추가한 컴포넌트들의 Initialize 호출
        GameObject::Initialize();
    }
    bool NPC::Setup(int spawnId, int npcId)
    {
        if (m_transform == nullptr || m_animator == nullptr)
            return false;

        // 같은 NPC 객체에 중복 등록하지 않도록 방지
        if (m_animator->FindAnimation(L"idle") != nullptr)
            return false;

        const NPCData* data =SingletonBase<NPCDataManager>::getInstance()->FindNPCData(npcId);

        if (data == nullptr)
            return false;

        auto idleIt = data->animations.find("idle");

        if (idleIt == data->animations.end())
            return false;

        const NPCAnimationData& idle = idleIt->second;

        if (idle.frames.empty())
            return false;

        auto* resourceManager = SingletonBase<ResourceManager>::getInstance();

        std::vector<Texture*> textures;
        std::vector<float> durations;

        textures.reserve(idle.frames.size());
        durations.reserve(idle.frames.size());

        for (const NPCAnimationFrame& frame : idle.frames)
        {
            if (!std::isfinite(frame.durationMs) || frame.durationMs <= 0.0f)
            {
                return false;
            }

            std::string key = frame.image;

            // 경로 구분자를 '/'로 통일
            for (char& ch : key)
            {
                if (ch == '\\')
                    ch = '/';
            }

            // Resources_Woodland/NPC/herbalist/idle_0.png
            // → NPC/herbalist/idle_0
            const std::string prefix = "Resources_Woodland/";

            if (key.compare(0, prefix.size(), prefix) != 0)
                return false;

            key.erase(0, prefix.size());

            if (key.size() < 4 || key.compare(key.size() - 4, 4, ".png") != 0)
            {
                return false;
            }

            key.resize(key.size() - 4);

            const std::wstring textureKey = Convert::Utf8ToWstr(key);

            Texture* texture = resourceManager->Find<Texture>(textureKey);

            if (texture == nullptr || texture->GetD2DBitmap() == nullptr)
            {
                OutputDebugStringW((L"[NPC] Texture unavailable: " + textureKey + L"\n").c_str());
                return false;
            }

            // 현재는 원본 크기 그대로 렌더링합니다.
            if (static_cast<int>(texture->GetWidth())
                != data->render.width
                || static_cast<int>(texture->GetHeight())
                != data->render.height)
            {
                return false;
            }

            textures.push_back(texture);
            durations.push_back(frame.durationMs / 1000.0f);
        }

        m_animator->CreateFrameAnimation(
            L"idle",
            textures,
            math::Vector2(
                data->render.originX,
                data->render.originY),
            math::Vector2::Zero,
            durations.front());

        Animation* animation = m_animator->FindAnimation(L"idle");

        if (animation == nullptr || !animation->SetFrameDurations(durations))
        {
            return false;
        }

        m_data = *data;
        m_spawnId = spawnId;

        m_transform->SetScale(math::Vector2(1.0f, 1.0f));

        // 원본 NPC 이미지는 왼쪽을 바라본다.
        // 맵 왼쪽의 NPC만 반전하여 NPC들이 중앙을 바라보게 한다.
        float mapCenterX = 768.0f;

        if (render::mainCamera != nullptr)
        {
            const float worldWidth = render::mainCamera->GetWorldSize().x;

            if (std::isfinite(worldWidth) && worldWidth > 0.0f)
                mapCenterX = worldWidth * 0.5f;
        }

        m_animator->SetFlipX(m_transform->GetPosition().x < mapCenterX);
        m_animator->PlayAnimation(L"idle", idle.loop);

        return true;
    }

    stb::NPC::~NPC()
    {
        delete m_animator;
        delete m_transform;
    }
}
