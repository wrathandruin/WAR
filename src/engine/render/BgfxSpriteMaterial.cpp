#include "engine/render/BgfxSpriteMaterial.h"

namespace war
{
    BgfxUvRect BgfxSpriteMaterials::uvFor(BgfxSpriteMaterialId material)
    {
        switch (material)
        {
        case BgfxSpriteMaterialId::FloorA:
            return { 0.0f / 5.0f, 0.0f / 2.0f, 1.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::FloorB:
            return { 1.0f / 5.0f, 0.0f / 2.0f, 2.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::FloorC:
            return { 2.0f / 5.0f, 0.0f / 2.0f, 3.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::WallA:
            return { 3.0f / 5.0f, 0.0f / 2.0f, 4.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::WallB:
            return { 4.0f / 5.0f, 0.0f / 2.0f, 5.0f / 5.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::WallC:
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
        case BgfxSpriteMaterialId::FloorA:
            return "FloorA";

        case BgfxSpriteMaterialId::FloorB:
            return "FloorB";

        case BgfxSpriteMaterialId::FloorC:
            return "FloorC";

        case BgfxSpriteMaterialId::WallA:
            return "WallA";

        case BgfxSpriteMaterialId::WallB:
            return "WallB";

        case BgfxSpriteMaterialId::WallC:
            return "WallC";

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
