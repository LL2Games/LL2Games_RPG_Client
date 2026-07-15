#pragma once
#include "CommonInclude.h"


enum class SkillType
{
    NONE,
    BASIC,
    MELEE_ARC,
    ENUMEND
};

struct SkillData
{
	int skillId;
    std::string key;
    int coolTimeMs;
	int mp_cost;
    SkillType type;
	float range;
	float angle_deg;

    std::string skillAnim_name;
    std::string skillVfx_name;
    std::string skillSfx_name;
    std::string name;
    std::string explain;
    std::string iconImg_name;

};

namespace Skill
{
    inline SkillType SetSkillType(std::string skillType)
    {
        if (skillType == "BASIC") return SkillType::BASIC;
        if (skillType == "MELEE_ARC") return SkillType::MELEE_ARC;
        else return SkillType::NONE;
    }
}
