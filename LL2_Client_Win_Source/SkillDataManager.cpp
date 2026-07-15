#include "SkillDataManager.h"
#include <fstream>

#define SKILL_PATH "Data/Skills/"
namespace fs = std::filesystem;

bool SkillDataManager::Init()
{
    if (!PreLoadAll()) return false;

    return true;
}

bool SkillDataManager::PreLoadAll()
{
    if (!fs::exists(SKILL_PATH))
    {
        printf("no exist %s\n", SKILL_PATH);
        return false;
    }
    for (const auto& entry : fs::recursive_directory_iterator(SKILL_PATH))
    {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        // 파일명에서 id 추출 (예: 2000000.json)
        int skill_id = 0;
        try {
            skill_id = std::stoi(entry.path().stem().string());
        }
        catch (...) {
            continue;
        }

        if (m_skillDatas.find(skill_id) != m_skillDatas.end())
            continue;

        SkillData skillData{};
        if (!LoadJsonFile(entry.path().string(), skillData))
        {
            return false;
        }
        m_skillDatas.emplace(skill_id, skillData);
    }

    return true;
}
bool SkillDataManager::LoadJsonFile(const std::string& path, SkillData& skillData)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    }
    catch (const nlohmann::json::parse_error&) {
        return false;
    }
    if (j.is_null()) return false;

    skillData.skillId       = j.at("skill_id").get<int>();
    skillData.key           = j.at("key").get<std::string>();
    skillData.coolTimeMs    = j.at("cooldown_ms").get<int>();
    skillData.mp_cost       = j.at("mp_cost").get<int>();
    skillData.type          = Skill::SetSkillType(j.at("type").get<std::string>());
    skillData.range         = j.at("range").get<int>();
    skillData.angle_deg     = j.at("angle_deg").get<int>();

    
    const auto& ue = j.at("client").at(0);

    skillData.skillAnim_name    = ue.at("anim").get<std::string>();
    skillData.skillVfx_name     = ue.at("vfx").get<std::string>();
    skillData.skillSfx_name     = ue.at("sfx").get<std::string>();
    skillData.iconImg_name      = ue.at("icon").get<std::string>();
    skillData.name              = ue.at("name").get<std::string>();
    skillData.explain           = ue.at("desc").get<std::string>();

    return true;
}


const SkillData* SkillDataManager::FindItemData(int skillId) const
{
	auto skillDataIter = m_skillDatas.find(skillId);
	if (skillDataIter == m_skillDatas.end()) return nullptr;

	return &skillDataIter->second;
}

