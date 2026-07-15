#pragma once
#include <string>
#include "stbMath.h"
#include "playerInfo.h"
#include "EquipeTypes.h"

struct AnimationEventInfo
{
	std::string start;
	std::string complete;
	std::string end;
};

struct AnimationInfo
{
	std::string anim_name;
	std::string frame_prefix;
	std::string path;
	int frame_count;
	int delay_ms;

	AnimationEventInfo animationEvent;
};

struct PlayerAnimationInfo
{
	std::string animName;
	std::string path;
	std::string framePrefix;
	int frameCount = 0;
	float delayMs = 0;

	std::vector<stb::math::Vector2> frameOffsets;

	AnimationEventInfo animationEvent;
};

struct PlayerAnimationKey
{
	JobType jobType;
	WeaponType weaponType;

	bool operator==(const PlayerAnimationKey& other) const
	{
		return jobType == other.jobType
			&& weaponType == other.weaponType;
	}
};

struct RenderInfo
{
	int width;
	int height;
	stb::math::Vector2 origin = stb::math::Vector2::Zero;
	stb::math::Vector2 offset = stb::math::Vector2::Zero;
};

struct PlayerAnimationKeyHash
{
	std::size_t operator()(const PlayerAnimationKey& key) const
	{
		return (static_cast<std::size_t>(key.jobType) << 16)
			^ static_cast<std::size_t>(key.weaponType);
	}
};

struct PlayerAnimationSet
{
	PlayerAnimationKey key;
	RenderInfo renderInfo{};
	std::vector<PlayerAnimationInfo> animations;
};




