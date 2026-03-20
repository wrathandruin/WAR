
#pragma once
#include <string>
#include "engine/world/WorldGrid.h"

namespace war
{
    struct Entity
    {
        int id{};
        std::string name;
        TileCoord tile{};
    };
}
