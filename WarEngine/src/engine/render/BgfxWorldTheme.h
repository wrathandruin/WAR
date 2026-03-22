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

    enum class BgfxThemePaletteMode
    {
        Default,
        Muted,
        Vivid
    };

    class BgfxWorldTheme
    {
    public:
        [[nodiscard]] static BgfxSpriteMaterialId floorMaterial(BgfxWorldThemeId theme);
        [[nodiscard]] static BgfxSpriteMaterialId wallMaterial(BgfxWorldThemeId theme);

        [[nodiscard]] static uint32_t floorTint(
            BgfxWorldThemeId theme,
            BgfxThemePaletteMode paletteMode,
            int tileVariant);

        [[nodiscard]] static uint32_t wallTint(
            BgfxWorldThemeId theme,
            BgfxThemePaletteMode paletteMode,
            int tileVariant);

        [[nodiscard]] static uint32_t regionBoundaryColor(
            BgfxWorldThemeId theme,
            BgfxThemePaletteMode paletteMode);

        [[nodiscard]] static const char* debugName(BgfxWorldThemeId theme);
        [[nodiscard]] static const char* debugName(BgfxThemePaletteMode paletteMode);
    };
}
