#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "Skill_Info.h"
#include <nlohmann/json.hpp>

class SkillDataManager : public stb::SingletonBase<SkillDataManager>
{
public:
    bool Init();
    bool PreLoadAll();
    bool LoadJsonFile(const std::string& path, SkillData& skillData);

public:
    const SkillData* FindItemData(int skillId) const;

private:

    std::unordered_map<int, SkillData> m_skillDatas;
};

