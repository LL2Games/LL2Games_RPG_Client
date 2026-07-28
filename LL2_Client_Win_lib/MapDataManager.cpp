#include "MapDataManager.h"
#include <fstream>

#define ITEM_PATH "Data/Maps/"
namespace fs = std::filesystem;

bool MapDataManager::Init()
{
    if (!PreLoadAll()) return false;

    return true;
}

bool MapDataManager::PreLoadAll()
{
    if (!fs::exists(ITEM_PATH))
    {
        std::string curPath = fs::current_path().string();
        printf("no exist %s\n", ITEM_PATH);
        return false;
    }

    for (const auto& entry : fs::recursive_directory_iterator(ITEM_PATH))
    {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        // 파일명에서 id 추출 (예: 2000000.json)
        int map_id = 0;
        try {
            map_id = std::stoi(entry.path().stem().string());
        }
        catch (...) {
            continue;
        }

        if (m_mapDatas.find(map_id) != m_mapDatas.end())
            continue;

        MapData mapData{};
        if (!LoadJsonFile(entry.path().string(), mapData))
        {
            return false;
        }
        m_mapDatas.emplace(map_id, mapData);
    }

    return true;

}

bool MapDataManager::LoadJsonFile(const std::string& path, MapData& mapData)
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

    mapData.mapId = j.at("mapId").get<int>();
    mapData.name = j.at("name").get<std::string>();
    mapData.background = j.at("background").get<std::string>();
  
    if (j.contains("portals"))
    {
        for (const auto& portalJson : j.at("portals"))
        {
            PortalData portalData;

            portalData.id = portalJson.value("id", "");
            portalData.texture = portalJson.value("texture", "");
            const auto& position = portalJson.at("position");
            portalData.position.x = position.value("x", 0.0f);
            portalData.position.y = position.value("y", 0.0f);
            if (portalJson.contains("renderSize"))
            {
                const auto& renderSize =
                    portalJson.at("renderSize");

                portalData.renderSize.x =
                    renderSize.value("x", 100.0f);

                portalData.renderSize.y =
                    renderSize.value("y", 160.0f);
            }

            const auto& halfSize = portalJson.at("halfSize");
            portalData.halfSize.x = halfSize.value("x", 60.0f);
            portalData.halfSize.y = halfSize.value("y", 100.0f);
            portalData.destinationMapId = portalJson.at("destinationMapId").get<int>();
            const auto& spawnPosition = portalJson.at("spawnPosition");
            portalData.spawnPosition.x = spawnPosition.value("x", 0.0f);
            portalData.spawnPosition.y = spawnPosition.value("y", 0.0f);

            mapData.portals.emplace_back(portalData);
        }
    }
    return true;
}

const MapData* MapDataManager::FindMapData(int mapId) const
{
    auto mapData = m_mapDatas.find(mapId);
    if (mapData == m_mapDatas.end())
    {
        return nullptr;
    }
    return &mapData->second;
}
