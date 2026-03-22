#include "engine/render/BgfxSpriteMaterial.h"

namespace war
{
    BgfxUvRect BgfxSpriteMaterials::uvFor(BgfxSpriteMaterialId material)
    {
        switch (material)
        {
        case BgfxSpriteMaterialId::IndustrialFloor:
            return { 0.0f / 5.0f, 0.0f / 2.0f, 1.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::SterileFloor:
            return { 1.0f / 5.0f, 0.0f / 2.0f, 2.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::EmergencyFloor:
            return { 2.0f / 5.0f, 0.0f / 2.0f, 3.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::IndustrialWall:
            return { 3.0f / 5.0f, 0.0f / 2.0f, 4.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::SterileWall:
            return { 4.0f / 5.0f, 0.0f / 2.0f, 5.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::EmergencyWall:
            return { 0.0f / 5.0f, 1.0f / 2.0f, 1.0f / 5.0f, 2.0f / 2.0f };

        case BgfxSpriteMaterialId::Player:
            return { 1.0f / 5.0f, 1.0f / 2.0f, 2.0f / 5.0f, 2.0f / 2.0f };

        case BgfxSpriteMaterialId::Crate:
            return { 2.0f / 5.0f, 1.0f / 2.0f, 3.0f / 5.0f, 2.0f / 2.0f };

        case BgfxSpriteMaterialId::Terminal:
            return { 3.0f / 5.0f, 1.0f / 2.0f, 4.0f / 5.0f, 2.0f / 2.0f };

        case BgfxSpriteMaterialId::Locker:
            return { 4.0f / 5.0f, 1.0f / 2.0f, 5.0f / 5.0f, 2.0f / 2.0f };

        default:
            return { 0.0f / 5.0f, 0.0f / 2.0f, 1.0f / 5.0f, 1.0f / 2.0f };
        }
    }

    const char* BgfxSpriteMaterials::debugName(BgfxSpriteMaterialId material)
    {
        switch (material)
        {
        case BgfxSpriteMaterialId::IndustrialFloor:
            return "IndustrialFloor";

        case BgfxSpriteMaterialId::SterileFloor:
            return "SterileFloor";

        case BgfxSpriteMaterialId::EmergencyFloor:
            return "EmergencyFloor";

        case BgfxSpriteMaterialId::IndustrialWall:
            return "IndustrialWall";

        case BgfxSpriteMaterialId::SterileWall:
            return "SterileWall";

        case BgfxSpriteMaterialId::EmergencyWall:
            return "EmergencyWall";

        case BgfxSpriteMaterialId::Player:
            return "Player";

        case BgfxSpriteMaterialId::Crate:
            return "Crate";

        case BgfxSpriteMaterialId::Terminal:
            return "Terminal";

        case BgfxSpriteMaterialId::Locker:
            return "Locker";

        default:
            return "Unknown";
        }
    }
}
