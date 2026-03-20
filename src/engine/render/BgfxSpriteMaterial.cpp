#include "engine/render/BgfxSpriteMaterial.h"

namespace war
{
    BgfxUvRect BgfxSpriteMaterials::uvFor(BgfxSpriteMaterialId material)
    {
        switch (material)
        {
        case BgfxSpriteMaterialId::Floor:
            return { 0.0f / 3.0f, 0.0f / 2.0f, 1.0f / 3.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::Wall:
            return { 1.0f / 3.0f, 0.0f / 2.0f, 2.0f / 3.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::Player:
            return { 2.0f / 3.0f, 0.0f / 2.0f, 3.0f / 3.0f, 1.0f / 2.0f };

        case BgfxSpriteMaterialId::Crate:
            return { 0.0f / 3.0f, 1.0f / 2.0f, 1.0f / 3.0f, 2.0f / 2.0f };

        case BgfxSpriteMaterialId::Terminal:
            return { 1.0f / 3.0f, 1.0f / 2.0f, 2.0f / 3.0f, 2.0f / 2.0f };

        case BgfxSpriteMaterialId::Locker:
            return { 2.0f / 3.0f, 1.0f / 2.0f, 3.0f / 3.0f, 2.0f / 2.0f };

        default:
            return { 0.0f / 3.0f, 0.0f / 2.0f, 1.0f / 3.0f, 1.0f / 2.0f };
        }
    }

    const char* BgfxSpriteMaterials::debugName(BgfxSpriteMaterialId material)
    {
        switch (material)
        {
        case BgfxSpriteMaterialId::Floor:
            return "Floor";

        case BgfxSpriteMaterialId::Wall:
            return "Wall";

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
