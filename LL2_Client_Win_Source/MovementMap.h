#pragma once
#include "MovementTypes.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace movement
{
    struct Platform { int id = 0; float left = 0, right = 0, y = 0; };
    struct Climbable
    {
        int id = 0;
        bool ladder = false;
        float x = 0, top = 0, bottom = 0, grabRange = 0;
        int topPlatformId = 0;
    };
    struct MapGeometry
    {
        float minX = 0, maxX = 0, killY = 0;
        Point safeFeet;
        std::vector<Platform> platforms;
        std::vector<Climbable> climbables;
        const Climbable* FindClimbable(int id) const
        {
            for (const auto& c : climbables) if (c.id == id) return &c;
            return nullptr;
        }
    };
    inline MapGeometry ParseMapGeometry(const nlohmann::json& json)
    {
        MapGeometry result;
        auto number = [](const nlohmann::json& object, const char* name)
        {
            float value = object.at(name).get<float>();
            if (!std::isfinite(value)) throw std::runtime_error("non-finite map geometry");
            return value;
        };
        result.minX = number(json, "minX"); result.maxX = number(json, "maxX");
        result.killY = number(json, "killY");
        result.safeFeet = {number(json.at("safeFeet"), "x"), number(json.at("safeFeet"), "y")};
        if (result.minX >= result.maxX || result.safeFeet.x < result.minX ||
            result.safeFeet.x > result.maxX || result.safeFeet.y >= result.killY)
            throw std::runtime_error("invalid map bounds/safeFeet");
        std::set<int> ids;
        for (const auto& p : json.at("platforms"))
        {
            Platform platform{p.at("id").get<int>(), number(p, "left"), number(p, "right"), number(p, "y")};
            if (platform.id <= 0 || !ids.insert(platform.id).second || platform.left >= platform.right)
                throw std::runtime_error("invalid platform");
            result.platforms.push_back(platform);
        }
        ids.clear();
        for (const auto& c : json.at("climbables"))
        {
            const std::string kind = c.at("kind").get<std::string>();
            Climbable climbable{c.at("id").get<int>(), kind == "ladder", number(c, "x"),
                number(c, "top"), number(c, "bottom"), number(c, "grabRange"), c.at("topPlatformId").get<int>()};
            if ((kind != "rope" && kind != "ladder") || climbable.id <= 0 ||
                !ids.insert(climbable.id).second || climbable.top >= climbable.bottom || climbable.grabRange <= 0)
                throw std::runtime_error("invalid climbable");
            bool exitFound = false;
            for (const auto& p : result.platforms)
                if (p.id == climbable.topPlatformId && climbable.x >= p.left && climbable.x <= p.right &&
                    std::abs(climbable.top - p.y) <= 0.05f) exitFound = true;
            if (!exitFound) throw std::runtime_error("climbable top platform missing");
            result.climbables.push_back(climbable); // Preserve server array order.
        }
        return result;
    }
}
