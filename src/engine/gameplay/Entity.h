#pragma once

#include <string>

#include "engine/world/WorldGrid.h"

namespace war
{
    enum class EntityType
    {
        Crate,
        Terminal,
        Locker
    };

    struct Entity
    {
        int id{};
        std::string name;
        EntityType type{};
        TileCoord tile{};
    };
}
