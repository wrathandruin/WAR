#pragma once

#include "engine/render/BgfxWorldTheme.h"

namespace war
{
    enum class WorldRegionTagId
    {
        CargoBay,
        TransitSpine,
        MedLab,
        CommandDeck,
        HazardContainment
    };

    class WorldRegionTags
    {
    public:
        [[nodiscard]] static BgfxWorldThemeId themeFor(WorldRegionTagId tag);
        [[nodiscard]] static const char* debugName(WorldRegionTagId tag);
    };
}
