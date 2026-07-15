#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "MonsterInfo.h"
#include <nlohmann/json.hpp>

class MonsterDataManager : public stb::SingletonBase<MonsterDataManager>
{
public:
    bool Init();
    bool PreLoadAll();
    bool LoadJsonFile(const std::string& path, MonsterData& monsterData);

public:
    const MonsterData* FindMonsterData(int monsterId) const;
private:

    std::unordered_map<int, MonsterData> m_monsterDatas;

};

