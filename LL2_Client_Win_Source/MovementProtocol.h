#pragma once
#include "MovementTypes.h"
#include <string>
#include <vector>

namespace movement
{
    bool ParseSnapshot(const std::string& payload, Snapshot& result, std::string& error);
    bool BuildInputFields(int mapId, int epoch, int sequence, const Input& input,
                          std::vector<std::string>& fields);
}
