#pragma once

#include <cstdint>

#include "engine/render/BgfxSpriteMaterial.h"

namespace war
{
    enum class BgfxWorldThemeId
    {
        Industrial,
        Sterile,
        Emergency
    };

    class BgfxWorldTheme
    {
    public:
        [[nodiscard]] static BgfxSpriteMaterialId floorMaterial(BgfxWorldThemeId theme);
        [[nodiscard]] static BgfxSpriteMaterialId wallMaterial(BgfxWorldThemeId theme);

        [[nodiscard]] static uint32_t floorTint(BgfxWorldThemeId theme, int tileVariant);
        [[nodiscard]] static uint32_t wallTint(BgfxWorldThemeId theme, int tileVariant);

        [[nodiscard]] static const char* debugName(BgfxWorldThemeId theme);
    };
}
