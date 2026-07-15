#pragma once
#include <stbMath.h>
#include "stbEnums.h"

struct CollierInfo
{
	stb::enums::eColliderType colliderType;
	stb::math::Vector2 offset{};
	stb::math::Vector2 halfSize{};

};

