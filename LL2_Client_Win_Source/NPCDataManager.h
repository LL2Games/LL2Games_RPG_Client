#pragma once
#include "CommonInclude.h"
#include "NPC_info.h"
#include "stbSingletonBase.h"
#include <nlohmann/json.hpp>

class NPCDataManager : public stb::SingletonBase<NPCDataManager>
{
public:
    bool Init();
    bool PreLoadAll();
    bool LoadJsonFile(const std::string& path, NPCData& npcData);

public:
    const NPCData* FindNPCData(int itemId) const;
private:

    std::unordered_map<int, NPCData> m_npcDatas;
};

