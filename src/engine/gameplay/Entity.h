#pragma once

#include <string>

#include "engine/world/WorldGrid.h"

namespace war
{
    enum class EntityType
    {
        Crate,
        Terminal,
        Locker,
        Ship
    };

    struct Entity
    {
        int id{};
        std::string name;
        EntityType type{};
        TileCoord tile{};

        bool isOpen = false;
        bool isLocked = false;
        bool isPowered = false;

        std::string lootProfileId;
        bool lootClaimed = false;
    };
}
