#pragma once
#include "Config.h"
#include <string>
#include <map>

struct GameInfo {
    std::string name;
    float yaw;
};

static std::map<GameType, GameInfo> gameLibrary = {
    { GAME_LUX, { "LUX", 0.06f } },
    { GAME_VALORANT, { "VALORANT", 0.07f } },
    { GAME_CS2, { "CS2 / APEX", 0.022f } },
    { GAME_OVERWATCH, { "OVERWATCH 2", 0.0066f } }
};

inline float GetGameMultiplier(GameType type) {
    return gameLibrary[type].yaw * 0.033333f; 
}
