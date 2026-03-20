#include "engine/world/WorldRegionTag.h"

namespace war
{
    BgfxWorldThemeId WorldRegionTags::themeFor(WorldRegionTagId tag)
    {
        switch (tag)
        {
        case WorldRegionTagId::CargoBay:
            return BgfxWorldThemeId::Industrial;

        case WorldRegionTagId::TransitSpine:
            return BgfxWorldThemeId::Industrial;

        case WorldRegionTagId::MedLab:
            return BgfxWorldThemeId::Sterile;

        case WorldRegionTagId::CommandDeck:
            return BgfxWorldThemeId::Sterile;

        case WorldRegionTagId::HazardContainment:
            return BgfxWorldThemeId::Emergency;

        default:
            return BgfxWorldThemeId::Industrial;
        }
    }

    const char* WorldRegionTags::debugName(WorldRegionTagId tag)
    {
        switch (tag)
        {
        case WorldRegionTagId::CargoBay:
            return "CargoBay";

        case WorldRegionTagId::TransitSpine:
            return "TransitSpine";

        case WorldRegionTagId::MedLab:
            return "MedLab";

        case WorldRegionTagId::CommandDeck:
            return "CommandDeck";

        case WorldRegionTagId::HazardContainment:
            return "HazardContainment";

        default:
            return "Unknown";
        }
    }
}
