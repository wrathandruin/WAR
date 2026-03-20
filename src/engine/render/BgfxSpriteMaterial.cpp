#include "engine/render/BgfxSpriteMaterial.h"

namespace war
{
    BgfxUvRect BgfxSpriteMaterials::uvFor(BgfxSpriteMaterialId material)
    {
        switch (material)
        {
        case BgfxSpriteMaterialId::Player:
            return { 0.0f, 0.0f, 0.5f, 0.5f };

        case BgfxSpriteMaterialId::Crate:
            return { 0.5f, 0.0f, 1.0f, 0.5f };

        case BgfxSpriteMaterialId::Terminal:
            return { 0.0f, 0.5f, 0.5f, 1.0f };

        case BgfxSpriteMaterialId::Locker:
            return { 0.5f, 0.5f, 1.0f, 1.0f };

        default:
            return { 0.0f, 0.0f, 0.5f, 0.5f };
        }
    }

    const char* BgfxSpriteMaterials::debugName(BgfxSpriteMaterialId material)
    {
        switch (material)
        {
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
