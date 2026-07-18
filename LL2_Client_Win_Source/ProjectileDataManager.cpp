#include "ProjectileDataManager.h"
#include <fstream>
#include "stbEnums.h"
#include "Collider_Info.h"
#include "stbMath.h"

#define PROJECTILE_PATH "Data/Projectile/"
namespace fs = std::filesystem;

bool ProjectileDataManager::Init()
{
    if (!PreLoadAll()) return false;

    return true;
}
bool ProjectileDataManager::PreLoadAll()
{
    if (!fs::exists(PROJECTILE_PATH))
    {
        printf("no exist %s\n", PROJECTILE_PATH);
        return false;
    }
    for (const auto& entry : fs::recursive_directory_iterator(PROJECTILE_PATH))
    {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        // 파일명에서 id 추출 (예: 2000000.json)
        int projectile_id = 0;
        try {
            projectile_id = std::stoi(entry.path().stem().string());
        }
        catch (...) {
            continue;
        }

        if (m_projectileDatas.find(projectile_id) != m_projectileDatas.end())
            continue;

        MonsterProjectileData projectileData{};
        if (!LoadJsonFile(entry.path().string(), projectileData))
        {
            return false;
        }
        m_projectileDatas.emplace(projectile_id, projectileData);
    }

    return true;
}

bool ProjectileDataManager::LoadJsonFile(const std::string& path, MonsterProjectileData& projectileData)
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
    /*
    

struct MonsterProjectileData {
	int instanceId;
	int projectileId;
	int dirX;
	int dirY;
	stb::math::Vector2 pos;

    ColliderInfo colliderInfo{};
};
    */
    projectileData.instanceId = j.at("monster_id").get<int>();
    projectileData.projectileId = j.value("name", "Unknown");
    projectileData.dirX = j.value("moveSpeed", 1);
    projectileData.dirY = j.value("moveSpeed", 1);


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


        projectileData.animations.emplace_back(info);
    }

    const auto& render = j.at("render");
    projectileData.renderInfo.width = render.value("width", 0);
    projectileData.renderInfo.height = render.value("height", 0);

    const auto& origin = render.at("origin");
    projectileData.renderInfo.origin.x = origin.value("x", 0.0f);
    projectileData.renderInfo.origin.y = origin.value("y", 0.0f);

    const auto& collider = j.at("collider");
    projectileData.colliderInfo.colliderType =
        stb::enums::SetColliderType(collider.value("type", "None"));


    const auto& offset = collider.at("offset");
    projectileData.colliderInfo.offset.x = offset.value("x", 0.0f);
    projectileData.colliderInfo.offset.y = offset.value("y", 0.0f);

    const auto& half = collider.at("half");
    projectileData.colliderInfo.halfSize.x = half.value("w", 0.0f);
    projectileData.colliderInfo.halfSize.y = half.value("h", 0.0f);

    const auto& ui = j.at("ui").at("hp_bar_offset");
    projectileData.UIPos.x = ui.value("x", 0.0f);
    projectileData.UIPos.y = ui.value("y", 0.0f);

    const auto& isRanged = j.at("isRanged");
    if (isRanged)
    {
        const auto& projectile = j.at("projectile");
        projectileData.projectileData.instanceId = projectile.value("id", 0);

        const auto& projectileCollider = projectile.at("collider");

        projectileData.projectileData.colliderInfo.colliderType =
            stb::enums::SetColliderType(projectileCollider.value("type", "None"));

        const auto& projectileOffset = projectileCollider.at("offset");
        projectileData.projectileData.colliderInfo.offset.x = projectileOffset.value("x", 0.0f);
        projectileData.projectileData.colliderInfo.offset.y = projectileOffset.value("y", 0.0f);

        const auto& projectileHalf = projectileCollider.at("half");
        projectileData.projectileData.colliderInfo.halfSize.x = projectileHalf.value("w", 0.0f);
        projectileData.projectileData.colliderInfo.halfSize.y = projectileHalf.value("h", 0.0f);
    }

    return true;
}

const MonsterProjectileData* ProjectileDataManager::FindMonsterData(int projectileId) const
{
    auto projectile = m_projectileDatas.find(projectileId);

    if (projectile == m_projectileDatas.end())
        return nullptr;
    return &projectile->second;
}
