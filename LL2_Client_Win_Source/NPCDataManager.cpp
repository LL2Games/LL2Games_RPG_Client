#include "NPCDataManager.h"
#include <fstream>

#define NPC_PATH "Data/Npcs/"
namespace fs = std::filesystem;

bool NPCDataManager::Init()
{
    return PreLoadAll();
}
bool NPCDataManager::PreLoadAll()
{
    if (!fs::exists(NPC_PATH))
    {
        std::string curPath = fs::current_path().string();
        printf("no exist %s\n", NPC_PATH);
        return false;
    }

    for (const auto& entry : fs::recursive_directory_iterator(NPC_PATH))
    {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        // 파일명에서 id 추출 (예: 2000000.json)
        int item_id = 0;
        try {
            item_id = std::stoi(entry.path().stem().string());
        }
        catch (...) {
            continue;
        }

        if (m_npcDatas.find(item_id) != m_npcDatas.end())
            continue;

        NPCData itemData{};
        if (!LoadJsonFile(entry.path().string(), itemData))
        {
            return false;
        }
        m_npcDatas.emplace(item_id, itemData);
    }

    return true;
}
bool NPCDataManager::LoadJsonFile(const std::string& path, NPCData& npcData)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    try
    {
        nlohmann::json j;
        file >> j;

        if (!j.is_object())
            return false;

        // 읽기에 실패했을 때 기존 npcData가 일부만 변경되지 않도록
        // 임시 객체에 먼저 저장합니다.
        NPCData data{};

        data.npcId = j.at("npcId").get<int>();
        data.name = j.at("name").get<std::string>();
        data.role = j.at("role").get<std::string>();

        const auto& render = j.at("render");

        data.render.width = render.at("width").get<int>();
        data.render.height = render.at("height").get<int>();

        const auto& origin = render.at("origin");

        data.render.originX = origin.at("x").get<float>();
        data.render.originY = origin.at("y").get<float>();

        if (data.npcId <= 0
            || data.name.empty()
            || data.render.width <= 0
            || data.render.height <= 0
            || !std::isfinite(data.render.originX)
            || !std::isfinite(data.render.originY))
        {
            return false;
        }

        const auto& animations = j.at("animations");
        if (!animations.is_object())
            return false;

        for (auto it = animations.begin(); it != animations.end(); ++it)
        {
            const std::string& animationName = it.key();
            const auto& animationJson = it.value();

            NPCAnimationData animation{};
            animation.loop = animationJson.at("loop").get<bool>();

            const auto& frames = animationJson.at("frames");

            if (!frames.is_array() || frames.empty())
                return false;

            for (const auto& frameJson : frames)
            {
                NPCAnimationFrame frame{};

                frame.image = frameJson.at("image").get<std::string>();

                frame.durationMs = frameJson.at("durationMs").get<float>();

                if (frame.image.empty() || !std::isfinite(frame.durationMs) || frame.durationMs <= 0.0f)
                {
                    return false;
                }

                animation.frames.push_back(std::move(frame));
            }

            data.animations.emplace(animationName,std::move(animation));
        }

        // NPC의 기본 대기 애니메이션은 필수
        if (data.animations.find("idle") == data.animations.end())
            return false;

        npcData = std::move(data);
        return true;
    }
    catch (const nlohmann::json::exception&)
    {
        // JSON 문법 오류, 필수 키 누락, 자료형 불일치
        return false;
    }
}

const NPCData* NPCDataManager::FindNPCData(int npcId) const
{
    auto iter = m_npcDatas.find(npcId);

    if (iter == m_npcDatas.end())
        return nullptr;

    return &iter->second;
}
