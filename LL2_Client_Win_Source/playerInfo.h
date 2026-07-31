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

struct StatViewData
{
	std::string name;
	int job = 0;
	int level = 0;

	int hp = 0;
	int maxHp = 0;
	int mp = 0;
	int maxMp = 0;

	int str = 0;
	int dex = 0;
	int intel = 0;
	int luk = 0;
	int remainAp = 0;
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
	Skill_Slash,
	Skill_End,
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

	inline std::wstring IntToJobWstring(const int job)
	{
		
		switch (job)
		{
		case 1:
			return L"전사";
			break;
		case 2:
			return L"궁수";
			break;
		case 3:
			return L"도적";
			break;
		case 4:
			return L"마법사";
			break;
		default:
			break;
		}
		return L"";
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

	inline std::string IntToStringStatType(const int statType)
	{
		switch (statType) 
		{
		case 1: return "str";
		case 2: return "dex";
		case 3: return "intel";
		case 4: return "luck";
		default: return "";
		}
	}
}
