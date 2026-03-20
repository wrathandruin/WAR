#pragma once

#include "engine/world/WorldGrid.h"

namespace war
{
    enum class ActionType
    {
        Move,
        Inspect
    };

    struct Action
    {
        ActionType type{};
        TileCoord target{};
    };
}
