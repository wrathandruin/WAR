#include "engine/world/WorldRegionTag.h"

namespace war
{
    BgfxWorldThemeId WorldRegionTags::themeFor(WorldRegionTagId tag)
    {
        switch (tag)
        {
        case WorldRegionTagId::IndustrialZone:
            return BgfxWorldThemeId::Industrial;

        case WorldRegionTagId::SterileZone:
            return BgfxWorldThemeId::Sterile;

        case WorldRegionTagId::EmergencyZone:
            return BgfxWorldThemeId::Emergency;

        default:
            return BgfxWorldThemeId::Industrial;
        }
    }

    const char* WorldRegionTags::debugName(WorldRegionTagId tag)
    {
        switch (tag)
        {
        case WorldRegionTagId::IndustrialZone:
            return "IndustrialZone";

        case WorldRegionTagId::SterileZone:
            return "SterileZone";

        case WorldRegionTagId::EmergencyZone:
            return "EmergencyZone";

        default:
            return "Unknown";
        }
    }
}
