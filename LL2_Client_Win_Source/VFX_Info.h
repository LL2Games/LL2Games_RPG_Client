#pragma once
#pragma once

#include <string>
#include <vector>

#include "stbMath.h"

struct VFXAnimationData
{
    bool loop = false;
    float frameTime = 0.05f;

    stb::math::Vector2 origin = stb::math::Vector2::Zero;
    stb::math::Vector2 offset = stb::math::Vector2::Zero;
    stb::math::Vector2 attachOffset = stb::math::Vector2::Zero;
    std::vector<std::wstring> frames;
};

struct VFXData
{
    std::string key;

    VFXAnimationData charge;
    VFXAnimationData effect;
    VFXAnimationData hit;
};
