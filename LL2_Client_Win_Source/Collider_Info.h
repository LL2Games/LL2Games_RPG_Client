#pragma once
#include "stbMath.h"
#include "stbEnums.h"

struct ColliderInfo
{
	stb::enums::eColliderType colliderType = stb::enums::eColliderType::EnumsEnd;
	stb::math::Vector2 offset{};
	stb::math::Vector2 halfSize{};

	float radius = 0.0f;
};

