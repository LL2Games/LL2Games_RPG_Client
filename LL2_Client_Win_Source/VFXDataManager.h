#pragma once

#include "VFX_Info.h"

#include <unordered_map>
#include <string>
#include <filesystem>

class VFXDataManager
{
public:
    bool Init();
    bool PreLoadAll();

    bool LoadJsonFile(const std::string& path, VFXData& outData);
    const VFXData* FindVFX(const std::string& key) const;

private:
    std::unordered_map<std::string, VFXData> m_vfxDatas;
};
