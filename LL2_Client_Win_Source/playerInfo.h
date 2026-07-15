#pragma once
#include "CommonInclude.h"
#include "stbmath.h"

struct BaseStat
{
	int str;
	int dex;
	int intel;
	int luck;
};

struct DerivedStat
{
	int maxHp;
	int maxMp;
};

struct ExpStat
{
	int level;
	int64_t exp;
	int64_t need_exp;
};

struct PlayerIdentity
{
	int charId = 0;
	int accountId = 0;
};

struct PlayerProfile
{
	std::string name;
	int level = 0;
	int rootJob = 0;
	int job = 0;
};

struct PlayerLocation
{
	int mapId = 0;
	stb::math::Vector2 pos;
};

enum class PlayerState
{
	Idle,
	Alert,
	Walk,
	Jump,
	Prone,
	Rope,
	Ladder,
	Attack,
	Dead,
	None
};

enum class JobType
{
	None,
	Warrior,
	Archer,
	Thief,
	Mage
};


namespace PlayerTypeUtil
{
	inline JobType StringToJobType(const std::string& str)
	{
		if (str == "warrior")
			return JobType::Warrior;

		if (str == "archer")
			return JobType::Archer;

		if (str == "thief")
			return JobType::Thief;

		if (str == "mage")
			return JobType::Mage;

		return JobType::None;
	}

	inline PlayerState IntToState(const int state)
	{
		switch (state)
		{
		case 0: return PlayerState::Idle;
		case 1: return PlayerState::Alert;
		case 2: return PlayerState::Walk;
		case 3: return PlayerState::Jump;
		case 4: return PlayerState::Prone;
		case 5: return PlayerState::Rope;
		case 6: return PlayerState::Ladder;
		case 7: return PlayerState::Attack;
		case 8: return PlayerState::Dead;
		default: return PlayerState::Idle;
		
		}
	}
}