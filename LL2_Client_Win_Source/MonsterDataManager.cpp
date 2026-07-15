#include "MonsterDataManager.h"
#include <fstream>
#include "stbEnums.h"
#include "Collider_Info.h"
#include "stbMath.h"

#define MONSTER_PATH "Data/Monsters/"
namespace fs = std::filesystem;

bool MonsterDataManager::Init()
{
	if (!PreLoadAll()) return false;

	return true;
}
bool MonsterDataManager::PreLoadAll()
{
    if (!fs::exists(MONSTER_PATH))
    {
        printf("no exist %s\n", MONSTER_PATH);
        return false;
    }
    for (const auto& entry : fs::recursive_directory_iterator(MONSTER_PATH))
    {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        // 파일명에서 id 추출 (예: 2000000.json)
        int monster_id = 0;
        try {
            monster_id = std::stoi(entry.path().stem().string());
        }
        catch (...) {
            continue;
        }

        if (m_monsterDatas.find(monster_id) != m_monsterDatas.end())
            continue;

        MonsterData monsterData{};
        if (!LoadJsonFile(entry.path().string(), monsterData))
        {
            return false;
        }
        m_monsterDatas.emplace(monster_id, monsterData);
    }

    return true;
}

bool MonsterDataManager::LoadJsonFile(const std::string& path, MonsterData& monsterData)
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

    monsterData.monster_id = j.at("monster_id").get<int>();
    monsterData.name = j.value("name", "Unknown");
    monsterData.moveSpeed = j.value("moveSpeed", 1);


    const auto& anims = j.at("animations");

    // get은 필수 값 없어서는 안되는 값
    // value는 없어도 기본값으로 동작 가능할 때 사용
    for (auto& [animName, animJson] : anims.items())
    {
        AnimationInfo info;

        info.anim_name = animName;
        info.path = animJson.at("path").get<std::string>();
        info.frame_prefix = animJson.at("frame_prefix").get<std::string>();
        info.frame_count = animJson.value("frame_count", 1);
        info.delay_ms = animJson.value("delay_ms", 150);

        if (animJson.contains("event"))
        {
            const auto& events = animJson.at("event");

            info.animationEvent.start = events.value("start", "");
            info.animationEvent.complete = events.value("complete", "");
            info.animationEvent.end = events.value("end", "");
        }
        

        monsterData.animations.emplace_back(info);
    }
   
    const auto& render = j.at("render");
    monsterData.renderInfo.width = render.value("width", 0);
    monsterData.renderInfo.height = render.value("height", 0);

    const auto& origin = render.at("origin");
    monsterData.renderInfo.origin.x = origin.value("x", 0.0f);
    monsterData.renderInfo.origin.y = origin.value("y", 0.0f);

    const auto& collider = j.at("collider");
    monsterData.colliderInfo.colliderType =
        stb::enums::SetColliderType(collider.value("type", "None"));


    const auto& offset = collider.at("offset");
    monsterData.colliderInfo.offset.x = offset.value("x", 0.0f);
    monsterData.colliderInfo.offset.y = offset.value("y", 0.0f);

    const auto& half = collider.at("half");
    monsterData.colliderInfo.halfSize.x = half.value("w", 0.0f);
    monsterData.colliderInfo.halfSize.y = half.value("h", 0.0f);
    
    const auto& ui = j.at("ui").at("hp_bar_offset");
    monsterData.UIPos.x = ui.value("x", 0.0f);
    monsterData.UIPos.y = ui.value("y", 0.0f);

    return true;
}

const MonsterData* MonsterDataManager::FindMonsterData(int monsterId) const
{
    auto monster = m_monsterDatas.find(monsterId);

    if (monster == m_monsterDatas.end())
        return nullptr;
    return &monster->second;
}
