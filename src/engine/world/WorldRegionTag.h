#pragma once

#include "engine/render/BgfxWorldTheme.h"

namespace war
{
    enum class WorldRegionTagId
    {
        IndustrialZone,
        SterileZone,
        EmergencyZone
    };

    class WorldRegionTags
    {
    public:
        [[nodiscard]] static BgfxWorldThemeId themeFor(WorldRegionTagId tag);
        [[nodiscard]] static const char* debugName(WorldRegionTagId tag);
    };
}
