#pragma once

#include "stbMath.h"

struct PortalData
{
    std::string id;
    std::string texture;

    stb::math::Vector2 position;
    stb::math::Vector2 renderSize =
        stb::math::Vector2(100.0f, 160.0f);

    stb::math::Vector2 halfSize =
        stb::math::Vector2(60.0f, 100.0f);

    int destinationMapId = 0;
    stb::math::Vector2 spawnPosition;
};

struct MapData
{
    int mapId = 0;

    std::string name;
    std::string background;
    std::string miniMap;

    std::vector<PortalData> portals;
};
