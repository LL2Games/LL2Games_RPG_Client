#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct NPCAnimationFrame
{
    std::string image;
    float durationMs = 100.0f;
};

struct NPCAnimationData
{
    bool loop = true;
    std::vector<NPCAnimationFrame> frames;
};

struct NPCRenderData
{
    int width = 96;
    int height = 96;

    float originX = 48.0f;
    float originY = 92.0f;
};

struct NPCData
{
    int npcId = 0;

    std::string name;
    std::string role;

    NPCRenderData render;

    // "idle" 등 애니메이션 이름으로 조회
    std::unordered_map<std::string, NPCAnimationData> animations;
};

struct NPCSpawnInfo
{
    int spawnId = 0;    // 현재 맵에서 NPC를 구분하는 배치 ID
    int npcId = 0;      // NPCDataManager에서 조회할 NPC ID

    float xPos = 0.0f;  // 발 기준 월드 X 좌표
    float yPos = 0.0f;  // 발 기준 월드 Y 좌표
};


