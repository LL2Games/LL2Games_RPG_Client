#pragma once

#include "CommonInclude.h"

#include <memory>
#include <string>
#include <vector>

#include "VFX_Info.h"

class stbD2DRenderer;

namespace stb
{
    class GameObject;
    class Transform;
    class Animator;
}

struct ActiveSkillEffect
{
    stb::GameObject* object = nullptr;
    stb::Transform* transform = nullptr;
    stb::Animator* animator = nullptr;

    bool finished = false;

    ~ActiveSkillEffect();
};

class SkillEffectManager
{
public:
    SkillEffectManager() = default;
    ~SkillEffectManager() = default;

public:
    void Update();
    void Render(stbD2DRenderer& renderer);

    void PlayCharge(const std::string& vfxKey, const stb::math::Vector2& playerPosition, bool flipX);
    void PlayEffect(const std::string& vfxKey, const stb::math::Vector2& position,bool flipX);
    void PlayHit(const std::string& vfxKey, const stb::math::Vector2& position);

    void Clear();

private:
    bool CreateEffect(const VFXAnimationData& animData,const stb::math::Vector2& position,bool flipX);

private:
    std::vector<std::unique_ptr<ActiveSkillEffect>> m_effects;
};
