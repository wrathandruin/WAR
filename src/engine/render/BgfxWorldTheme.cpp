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

    uint32_t BgfxWorldTheme::floorTint(BgfxWorldThemeId theme, int tileVariant)
    {
        switch (theme)
        {
        case BgfxWorldThemeId::Industrial:
            return tileVariant == 0
                ? rgbaToAbgr(235, 235, 235)
                : rgbaToAbgr(255, 255, 255);

        case BgfxWorldThemeId::Sterile:
            return tileVariant == 0
                ? rgbaToAbgr(225, 245, 255)
                : rgbaToAbgr(245, 252, 255);

        case BgfxWorldThemeId::Emergency:
            return tileVariant == 0
                ? rgbaToAbgr(255, 230, 230)
                : rgbaToAbgr(255, 245, 245);

        default:
            return rgbaToAbgr(255, 255, 255);
        }
    }

    uint32_t BgfxWorldTheme::wallTint(BgfxWorldThemeId theme, int tileVariant)
    {
        switch (theme)
        {
        case BgfxWorldThemeId::Industrial:
            return tileVariant == 0
                ? rgbaToAbgr(208, 208, 208)
                : rgbaToAbgr(220, 220, 220);

        case BgfxWorldThemeId::Sterile:
            return tileVariant == 0
                ? rgbaToAbgr(215, 235, 245)
                : rgbaToAbgr(228, 245, 255);

        case BgfxWorldThemeId::Emergency:
            return tileVariant == 0
                ? rgbaToAbgr(255, 205, 205)
                : rgbaToAbgr(255, 220, 220);

        default:
            return rgbaToAbgr(220, 220, 220);
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
}
