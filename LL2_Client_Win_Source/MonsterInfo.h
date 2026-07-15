#pragma once
#include "stbMath.h"
#include "AnimationInfo.h"
#include "Collider_Info.h"

enum class MonsterState
{
	E_Idle,
	E_Patrol,
	E_Chase,
	E_Move,
	E_Die,
	E_Hit,
	E_NONE,
};

struct MonsterSpawnInfo
{
	int monsterId;
	int instanceId;
	
	int dir;
	int moveSpeed;
	int curHp;
	int maxHp;
	stb::math::Vector2 pos;
	MonsterState state;
};

struct MonsterUpdateInfo
{
	int instanceId;
	int monsterId = 0;
	int dir;
	int moveSpeed;
	int curHp;
	int maxHp;
	stb::math::Vector2 pos;
	MonsterState state;
};


struct MonsterData
{
	int monster_id = 0;
	std::string name;

	int moveSpeed = 0;
	stb::math::Vector2 UIPos = stb::math::Vector2::Zero;
	RenderInfo renderInfo{};
	std::vector<AnimationInfo> animations;

	CollierInfo colliderInfo{};
};



namespace monster
{
	inline MonsterState SetState(int state)
	{
		switch (state)
		{
		case 0: return MonsterState::E_Idle;
		case 1: return MonsterState::E_Patrol;
		case 2: return MonsterState::E_Chase;
		case 3: return MonsterState::E_Move;
		case 4: return MonsterState::E_Die;
		case 5: return MonsterState::E_Hit;
		default: return MonsterState::E_NONE;
		}
	}
}

