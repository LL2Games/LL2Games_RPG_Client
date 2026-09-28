#include "VFXDataManager.h"
#include "CommonInclude.h"
#include <fstream>
#include <nlohmann/json.hpp>

#define VFX_PATH "Data/VFX/"

namespace fs = std::filesystem;

static std::wstring Utf8ToWString(const std::string& str)
{
    if (str.empty())
        return L"";

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);

    std::wstring result(sizeNeeded, 0);

    MultiByteToWideChar(
        CP_UTF8,
        0,
        str.c_str(),
        (int)str.size(),
        result.data(),
        sizeNeeded
    );

    return result;
}

bool VFXDataManager::Init()
{
    return PreLoadAll();
}

bool VFXDataManager::PreLoadAll()
{
    if (!fs::exists(VFX_PATH))  
    {
        OutputDebugStringA("[VFXDataManager] VFX path not found\n");
        return false;
    }

    // 기존에 로드된 VFX 데이터 제거
    m_vfxDatas.clear();

    for (const auto& entry :fs::recursive_directory_iterator(VFX_PATH))
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != ".json")
            continue;   

        VFXData data{};

        if (!LoadJsonFile(entry.path().string(),data))
        {
            return false;
        }

        // 같은 key가 있어도 새 데이터로 교체
        m_vfxDatas.insert_or_assign(data.key,std::move(data));
    }

    return true;
}

bool VFXDataManager::LoadJsonFile(const std::string& path, VFXData& outData)
{
    std::ifstream file(path);

    if (!file.is_open())
        return false;

    nlohmann::json j;

    try
    {
        file >> j;

        outData = VFXData{};

        outData.key = j.at("key").get<std::string>();

        auto LoadAnimationData =
            [](const nlohmann::json& src, VFXAnimationData& dst)
            {
                dst.loop = src.value("loop", false);
                dst.frameTime = src.value("frame_time", 0.05f);

                if (src.contains("origin"))
                {
                    dst.origin.x = src["origin"].value("x", 0.0f);
                    dst.origin.y = src["origin"].value("y", 0.0f);
                }

                if (src.contains("offset"))
                {
                    dst.offset.x = src["offset"].value("x", 0.0f);
                    dst.offset.y = src["offset"].value("y", 0.0f);
                }

                if (src.contains("attach_offset"))
                {
                    dst.attachOffset.x = src["attach_offset"].value("x", 0.0f);
                    dst.attachOffset.y = src["attach_offset"].value("y", 0.0f);
                }

                if (src.contains("frames"))
                {
                    for (const auto& frame : src["frames"])
                    {
                        dst.frames.push_back(Utf8ToWString(frame.get<std::string>()));
                    }
                }
            };

        if (j.contains("charge"))
        {
            LoadAnimationData(j["charge"], outData.charge);
        }

        if (j.contains("effect"))
        {
            LoadAnimationData(j["effect"], outData.effect);
        }

        if (j.contains("hit"))
        {
            OutputDebugStringA("[VFXDataManager] hit block found\n");

            LoadAnimationData(j["hit"], outData.hit);
        }
    }
    catch (const std::exception& e)
    {
        OutputDebugStringA(("[VFXDataManager] JSON ERROR : " + std::string(e.what()) + "\n").c_str());
        return false;
    }

    return true;
}

const VFXData* VFXDataManager::FindVFX(const std::string& key) const
{
    auto it = m_vfxDatas.find(key);

    if (it == m_vfxDatas.end())
        return nullptr;

    return &it->second;
}
