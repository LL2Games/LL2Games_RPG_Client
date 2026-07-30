#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "MonsterInfo.h"
#include <nlohmann/json.hpp>

class ProjectileDataManager : public stb::SingletonBase<ProjectileDataManager>
{
public:
    bool Init();
    bool PreLoadAll();
    bool LoadJsonFile(const std::string& path, MonsterProjectileData& ProjectileData);

public:
    const MonsterProjectileData* FindMonsterData(int projectileId) const;
private:

    std::unordered_map<int, MonsterProjectileData> m_projectileDatas;

};

