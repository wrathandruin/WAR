#include "engine/render/BgfxWorldTheme.h"

namespace war
{
    namespace
    {
        uint32_t rgbaToAbgr(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
        {
            return (static_cast<uint32_t>(a) << 24)
                | (static_cast<uint32_t>(b) << 16)
                | (static_cast<uint32_t>(g) << 8)
                | static_cast<uint32_t>(r);
        }
    }

    BgfxSpriteMaterialId BgfxWorldTheme::floorMaterial(BgfxWorldThemeId theme)
    {
        switch (theme)
        {
        case BgfxWorldThemeId::Industrial:
            return BgfxSpriteMaterialId::IndustrialFloor;

        case BgfxWorldThemeId::Sterile:
            return BgfxSpriteMaterialId::SterileFloor;

        case BgfxWorldThemeId::Emergency:
            return BgfxSpriteMaterialId::EmergencyFloor;

        default:
            return BgfxSpriteMaterialId::IndustrialFloor;
        }
    }

    BgfxSpriteMaterialId BgfxWorldTheme::wallMaterial(BgfxWorldThemeId theme)
    {
        switch (theme)
        {
        case BgfxWorldThemeId::Industrial:
            return BgfxSpriteMaterialId::IndustrialWall;

        case BgfxWorldThemeId::Sterile:
            return BgfxSpriteMaterialId::SterileWall;

        case BgfxWorldThemeId::Emergency:
            return BgfxSpriteMaterialId::EmergencyWall;

        default:
            return BgfxSpriteMaterialId::IndustrialWall;
        }
    }

    uint32_t BgfxWorldTheme::floorTint(
        BgfxWorldThemeId theme,
        BgfxThemePaletteMode paletteMode,
        int tileVariant)
    {
        switch (theme)
        {
        case BgfxWorldThemeId::Industrial:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return tileVariant == 0
                    ? rgbaToAbgr(205, 210, 214)
                    : rgbaToAbgr(220, 224, 228);

            case BgfxThemePaletteMode::Vivid:
                return tileVariant == 0
                    ? rgbaToAbgr(245, 245, 248)
                    : rgbaToAbgr(255, 255, 255);

            default:
                return tileVariant == 0
                    ? rgbaToAbgr(235, 235, 235)
                    : rgbaToAbgr(255, 255, 255);
            }

        case BgfxWorldThemeId::Sterile:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return tileVariant == 0
                    ? rgbaToAbgr(205, 225, 235)
                    : rgbaToAbgr(220, 234, 242);

            case BgfxThemePaletteMode::Vivid:
                return tileVariant == 0
                    ? rgbaToAbgr(230, 248, 255)
                    : rgbaToAbgr(246, 252, 255);

            default:
                return tileVariant == 0
                    ? rgbaToAbgr(225, 245, 255)
                    : rgbaToAbgr(245, 252, 255);
            }

        case BgfxWorldThemeId::Emergency:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return tileVariant == 0
                    ? rgbaToAbgr(225, 196, 196)
                    : rgbaToAbgr(235, 208, 208);

            case BgfxThemePaletteMode::Vivid:
                return tileVariant == 0
                    ? rgbaToAbgr(255, 214, 214)
                    : rgbaToAbgr(255, 232, 232);

            default:
                return tileVariant == 0
                    ? rgbaToAbgr(255, 230, 230)
                    : rgbaToAbgr(255, 245, 245);
            }

        default:
            return rgbaToAbgr(255, 255, 255);
        }
    }

    uint32_t BgfxWorldTheme::wallTint(
        BgfxWorldThemeId theme,
        BgfxThemePaletteMode paletteMode,
        int tileVariant)
    {
        switch (theme)
        {
        case BgfxWorldThemeId::Industrial:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return tileVariant == 0
                    ? rgbaToAbgr(184, 184, 184)
                    : rgbaToAbgr(196, 196, 196);

            case BgfxThemePaletteMode::Vivid:
                return tileVariant == 0
                    ? rgbaToAbgr(220, 220, 220)
                    : rgbaToAbgr(235, 235, 235);

            default:
                return tileVariant == 0
                    ? rgbaToAbgr(208, 208, 208)
                    : rgbaToAbgr(220, 220, 220);
            }

        case BgfxWorldThemeId::Sterile:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return tileVariant == 0
                    ? rgbaToAbgr(194, 214, 224)
                    : rgbaToAbgr(208, 224, 232);

            case BgfxThemePaletteMode::Vivid:
                return tileVariant == 0
                    ? rgbaToAbgr(224, 244, 255)
                    : rgbaToAbgr(236, 248, 255);

            default:
                return tileVariant == 0
                    ? rgbaToAbgr(215, 235, 245)
                    : rgbaToAbgr(228, 245, 255);
            }

        case BgfxWorldThemeId::Emergency:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return tileVariant == 0
                    ? rgbaToAbgr(214, 168, 168)
                    : rgbaToAbgr(225, 182, 182);

            case BgfxThemePaletteMode::Vivid:
                return tileVariant == 0
                    ? rgbaToAbgr(255, 214, 214)
                    : rgbaToAbgr(255, 228, 228);

            default:
                return tileVariant == 0
                    ? rgbaToAbgr(255, 205, 205)
                    : rgbaToAbgr(255, 220, 220);
            }

        default:
            return rgbaToAbgr(220, 220, 220);
        }
    }

    uint32_t BgfxWorldTheme::regionBoundaryColor(
        BgfxWorldThemeId theme,
        BgfxThemePaletteMode paletteMode)
    {
        switch (theme)
        {
        case BgfxWorldThemeId::Industrial:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return rgbaToAbgr(118, 128, 138, 200);

            case BgfxThemePaletteMode::Vivid:
                return rgbaToAbgr(166, 184, 204, 255);

            default:
                return rgbaToAbgr(144, 158, 176, 228);
            }

        case BgfxWorldThemeId::Sterile:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return rgbaToAbgr(118, 188, 220, 200);

            case BgfxThemePaletteMode::Vivid:
                return rgbaToAbgr(110, 222, 255, 255);

            default:
                return rgbaToAbgr(128, 208, 244, 228);
            }

        case BgfxWorldThemeId::Emergency:
            switch (paletteMode)
            {
            case BgfxThemePaletteMode::Muted:
                return rgbaToAbgr(196, 112, 112, 200);

            case BgfxThemePaletteMode::Vivid:
                return rgbaToAbgr(255, 110, 110, 255);

            default:
                return rgbaToAbgr(236, 124, 124, 228);
            }

        default:
            return rgbaToAbgr(255, 255, 255, 220);
        }
    }

    const char* BgfxWorldTheme::debugName(BgfxWorldThemeId theme)
    {
        switch (theme)
        {
        case BgfxWorldThemeId::Industrial:
            return "Industrial";

        case BgfxWorldThemeId::Sterile:
            return "Sterile";

        case BgfxWorldThemeId::Emergency:
            return "Emergency";

        default:
            return "Unknown";
        }
    }

    const char* BgfxWorldTheme::debugName(BgfxThemePaletteMode paletteMode)
    {
        switch (paletteMode)
        {
        case BgfxThemePaletteMode::Default:
            return "Default";

        case BgfxThemePaletteMode::Muted:
            return "Muted";

        case BgfxThemePaletteMode::Vivid:
            return "Vivid";

        default:
            return "Unknown";
        }
    }
}
