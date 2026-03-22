#pragma once

#include "engine/world/WorldGrid.h"

namespace war
{
    enum class ActionType
    {
        Move,
        Inspect,
        Interact
    };

    struct Action
    {
        ActionType type{};
        TileCoord target{};
    };
}
