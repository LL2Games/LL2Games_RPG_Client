#pragma once

#include "..\\LL2_Client_Win_Source\\CommonInclude.h"
#include "..\\LL2_Client_Win_Source\\stbSingletonBase.h"
#include "MapInfo.h"
#include <nlohmann/json.hpp>

class MapDataManager : public stb::SingletonBase<MapDataManager>
{
public:
    bool Init();
    bool PreLoadAll();
    bool LoadJsonFile(const std::string& path,MapData& mapData);

    const MapData* FindMapData(int mapId) const;

private:
    std::unordered_map<int, MapData> m_mapDatas;
};

