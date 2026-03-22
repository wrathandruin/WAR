#pragma once

#include <cstdint>

#include "engine/render/BgfxSpriteMaterial.h"
#include "engine/world/WorldState.h"

namespace war
{
    class BgfxTileVisuals
    {
    public:
        [[nodiscard]] static BgfxSpriteMaterialId materialForTile(
            const WorldState& worldState,
            TileCoord tile);

        [[nodiscard]] static uint32_t tintForTile(
            const WorldState& worldState,
            TileCoord tile);
    };
}
