#include "engine/render/WorldRenderer.h"

#include <algorithm>

#include "engine/render/BgfxWorldTheme.h"

namespace war
{
    namespace
    {
        COLORREF entityFillColor(const WorldState& worldState, const Entity& entity)
        {
            switch (worldState.regionTag(entity.tile))
            {
            case WorldRegionTagId::CargoBay:
                switch (entity.type)
                {
                case EntityType::Crate: return entity.isLocked ? RGB(176, 120, 78) : RGB(212, 168, 104);
                case EntityType::Terminal: return entity.isPowered ? RGB(120, 176, 212) : RGB(96, 124, 148);
                case EntityType::Locker: return entity.isLocked ? RGB(188, 150, 118) : RGB(166, 166, 176);
                default: break;
                }
                break;

            case WorldRegionTagId::TransitSpine:
                switch (entity.type)
                {
                case EntityType::Crate: return RGB(192, 176, 128);
                case EntityType::Terminal: return entity.isPowered ? RGB(146, 214, 244) : RGB(114, 152, 180);
                case EntityType::Locker: return entity.isLocked ? RGB(212, 186, 124) : RGB(186, 190, 206);
                default: break;
                }
                break;

            case WorldRegionTagId::MedLab:
                switch (entity.type)
                {
                case EntityType::Crate: return RGB(214, 224, 232);
                case EntityType::Terminal: return entity.isPowered ? RGB(134, 236, 255) : RGB(118, 170, 188);
                case EntityType::Locker: return entity.isLocked ? RGB(210, 228, 240) : RGB(234, 242, 252);
                default: break;
                }
                break;

            case WorldRegionTagId::CommandDeck:
                switch (entity.type)
                {
                case EntityType::Crate: return RGB(188, 198, 216);
                case EntityType::Terminal: return entity.isPowered ? RGB(120, 188, 255) : RGB(92, 132, 186);
                case EntityType::Locker: return entity.isLocked ? RGB(220, 210, 166) : RGB(212, 220, 236);
                default: break;
                }
                break;

            case WorldRegionTagId::HazardContainment:
                switch (entity.type)
                {
                case EntityType::Crate: return entity.isLocked ? RGB(184, 96, 96) : RGB(158, 84, 84);
                case EntityType::Terminal: return entity.isPowered ? RGB(255, 152, 132) : RGB(164, 98, 98);
                case EntityType::Locker: return entity.isLocked ? RGB(232, 118, 118) : RGB(202, 116, 116);
                default: break;
                }
                break;

            default:
                break;
            }

            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen ? RGB(110, 110, 110) : RGB(120, 255, 150);

            case EntityType::Terminal:
                return entity.isPowered ? RGB(90, 170, 255) : RGB(70, 110, 150);

            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return RGB(220, 110, 110);
                }
                return entity.isOpen ? RGB(160, 160, 210) : RGB(190, 190, 240);

            default:
                return RGB(120, 255, 150);
            }
        }

        COLORREF entityOutlineColor(const WorldState& worldState, const Entity& entity)
        {
            switch (worldState.regionTag(entity.tile))
            {
            case WorldRegionTagId::CargoBay:
                switch (entity.type)
                {
                case EntityType::Crate: return RGB(235, 210, 160);
                case EntityType::Terminal: return RGB(170, 205, 228);
                case EntityType::Locker: return RGB(208, 190, 170);
                default: break;
                }
                break;

            case WorldRegionTagId::TransitSpine:
                switch (entity.type)
                {
                case EntityType::Crate: return RGB(225, 216, 170);
                case EntityType::Terminal: return RGB(196, 228, 246);
                case EntityType::Locker: return RGB(228, 214, 170);
                default: break;
                }
                break;

            case WorldRegionTagId::MedLab:
                switch (entity.type)
                {
                case EntityType::Crate: return RGB(240, 246, 250);
                case EntityType::Terminal: return RGB(194, 246, 255);
                case EntityType::Locker: return RGB(245, 250, 255);
                default: break;
                }
                break;

            case WorldRegionTagId::CommandDeck:
                switch (entity.type)
                {
                case EntityType::Crate: return RGB(220, 230, 246);
                case EntityType::Terminal: return RGB(182, 220, 255);
                case EntityType::Locker: return RGB(246, 236, 190);
                default: break;
                }
                break;

            case WorldRegionTagId::HazardContainment:
                switch (entity.type)
                {
                case EntityType::Crate: return RGB(220, 150, 150);
                case EntityType::Terminal: return RGB(255, 188, 168);
                case EntityType::Locker: return RGB(255, 170, 170);
                default: break;
                }
                break;

            default:
                break;
            }

            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen ? RGB(170, 170, 170) : RGB(200, 255, 210);

            case EntityType::Terminal:
                return entity.isPowered ? RGB(180, 220, 255) : RGB(120, 160, 190);

            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return RGB(255, 180, 180);
                }
                return entity.isOpen ? RGB(220, 220, 255) : RGB(240, 240, 255);

            default:
                return RGB(200, 255, 210);
            }
        }

        COLORREF entityStateAccentColor(const Entity& entity)
        {
            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen ? RGB(178, 178, 178) : RGB(232, 208, 124);

            case EntityType::Terminal:
                return entity.isPowered ? RGB(122, 238, 255) : RGB(112, 142, 170);

            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return RGB(255, 132, 132);
                }
                return entity.isOpen ? RGB(216, 224, 255) : RGB(204, 204, 224);

            default:
                return RGB(255, 255, 255);
            }
        }

        COLORREF tileFillColor(
            BgfxWorldThemeId theme,
            BgfxThemePaletteMode paletteMode,
            bool blocked,
            int tileVariant)
        {
            switch (theme)
            {
            case BgfxWorldThemeId::Industrial:
                if (blocked)
                {
                    switch (paletteMode)
                    {
                    case BgfxThemePaletteMode::Muted: return tileVariant == 0 ? RGB(184, 184, 184) : RGB(196, 196, 196);
                    case BgfxThemePaletteMode::Vivid: return tileVariant == 0 ? RGB(220, 220, 220) : RGB(235, 235, 235);
                    default: return tileVariant == 0 ? RGB(208, 208, 208) : RGB(220, 220, 220);
                    }
                }
                switch (paletteMode)
                {
                case BgfxThemePaletteMode::Muted: return tileVariant == 0 ? RGB(205, 210, 214) : RGB(220, 224, 228);
                case BgfxThemePaletteMode::Vivid: return tileVariant == 0 ? RGB(245, 245, 248) : RGB(255, 255, 255);
                default: return tileVariant == 0 ? RGB(235, 235, 235) : RGB(255, 255, 255);
                }

            case BgfxWorldThemeId::Sterile:
                if (blocked)
                {
                    switch (paletteMode)
                    {
                    case BgfxThemePaletteMode::Muted: return tileVariant == 0 ? RGB(194, 214, 224) : RGB(208, 224, 232);
                    case BgfxThemePaletteMode::Vivid: return tileVariant == 0 ? RGB(224, 244, 255) : RGB(236, 248, 255);
                    default: return tileVariant == 0 ? RGB(215, 235, 245) : RGB(228, 245, 255);
                    }
                }
                switch (paletteMode)
                {
                case BgfxThemePaletteMode::Muted: return tileVariant == 0 ? RGB(205, 225, 235) : RGB(220, 234, 242);
                case BgfxThemePaletteMode::Vivid: return tileVariant == 0 ? RGB(230, 248, 255) : RGB(246, 252, 255);
                default: return tileVariant == 0 ? RGB(225, 245, 255) : RGB(245, 252, 255);
                }

            case BgfxWorldThemeId::Emergency:
                if (blocked)
                {
                    switch (paletteMode)
                    {
                    case BgfxThemePaletteMode::Muted: return tileVariant == 0 ? RGB(214, 168, 168) : RGB(225, 182, 182);
                    case BgfxThemePaletteMode::Vivid: return tileVariant == 0 ? RGB(255, 214, 214) : RGB(255, 228, 228);
                    default: return tileVariant == 0 ? RGB(255, 205, 205) : RGB(255, 220, 220);
                    }
                }
                switch (paletteMode)
                {
                case BgfxThemePaletteMode::Muted: return tileVariant == 0 ? RGB(225, 196, 196) : RGB(235, 208, 208);
                case BgfxThemePaletteMode::Vivid: return tileVariant == 0 ? RGB(255, 214, 214) : RGB(255, 232, 232);
                default: return tileVariant == 0 ? RGB(255, 230, 230) : RGB(255, 245, 245);
                }

            default:
                return blocked ? RGB(220, 220, 220) : RGB(255, 255, 255);
            }
        }

        COLORREF tileOutlineColor(BgfxWorldThemeId theme)
        {
            switch (theme)
            {
            case BgfxWorldThemeId::Industrial:
                return RGB(95, 105, 118);

            case BgfxWorldThemeId::Sterile:
                return RGB(108, 164, 192);

            case BgfxWorldThemeId::Emergency:
                return RGB(176, 92, 92);

            default:
                return RGB(80, 80, 80);
            }
        }

        COLORREF regionBoundaryColor(BgfxWorldThemeId theme, BgfxThemePaletteMode paletteMode)
        {
            switch (theme)
            {
            case BgfxWorldThemeId::Industrial:
                switch (paletteMode)
                {
                case BgfxThemePaletteMode::Muted: return RGB(118, 128, 138);
                case BgfxThemePaletteMode::Vivid: return RGB(166, 184, 204);
                default: return RGB(144, 158, 176);
                }

            case BgfxWorldThemeId::Sterile:
                switch (paletteMode)
                {
                case BgfxThemePaletteMode::Muted: return RGB(118, 188, 220);
                case BgfxThemePaletteMode::Vivid: return RGB(110, 222, 255);
                default: return RGB(128, 208, 244);
                }

            case BgfxWorldThemeId::Emergency:
                switch (paletteMode)
                {
                case BgfxThemePaletteMode::Muted: return RGB(196, 112, 112);
                case BgfxThemePaletteMode::Vivid: return RGB(255, 110, 110);
                default: return RGB(236, 124, 124);
                }

            default:
                return RGB(255, 255, 255);
            }
        }

        COLORREF hotspotMarkerColor(const WorldAuthoringHotspot& hotspot)
        {
            switch (hotspot.type)
            {
            case WorldAuthoringHotspotType::Encounter:
                return hotspot.encounterReady ? RGB(255, 214, 112) : RGB(220, 186, 98);
            case WorldAuthoringHotspotType::Control:
                return hotspot.encounterReady ? RGB(128, 198, 255) : RGB(102, 160, 220);
            case WorldAuthoringHotspotType::Transit:
                return hotspot.encounterReady ? RGB(130, 240, 255) : RGB(112, 208, 226);
            case WorldAuthoringHotspotType::Loot:
                return hotspot.encounterReady ? RGB(242, 228, 150) : RGB(216, 198, 126);
            case WorldAuthoringHotspotType::Hazard:
                return hotspot.encounterReady ? RGB(255, 146, 146) : RGB(220, 124, 124);
            default:
                return RGB(255, 255, 255);
            }
        }

        COLORREF hoveredOutlineColor(const WorldState& worldState, TileCoord tile)
        {
            if (!worldState.world().isInBounds(tile))
            {
                return RGB(190, 190, 190);
            }

            if (worldState.world().isBlocked(tile))
            {
                return RGB(232, 120, 120);
            }

            const Entity* entity = worldState.entities().getAt(tile);
            if (entity != nullptr)
            {
                return RGB(132, 228, 255);
            }

            const WorldAuthoringHotspot* hotspot = worldState.authoringHotspotAt(tile);
            if (hotspot != nullptr)
            {
                return hotspotMarkerColor(*hotspot);
            }

            return RGB(242, 226, 122);
        }

        int tileVariant(TileCoord tile)
        {
            const int value = tile.x * 17 + tile.y * 31 + tile.x * tile.y * 3;
            const int hash = value < 0 ? -value : value;
            return hash % 2;
        }
    }

    void WorldRenderer::render(
        HDC dc,
        const RECT& clientRect,
        const WorldState& worldState,
        const Camera2D& camera,
        const Vec2& playerPosition,
        const std::vector<RemotePresenceRenderProxy>& remotePresenceProxies,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex,
        bool hasHoveredTile,
        TileCoord hoveredTile,
        bool hasSelectedTile,
        TileCoord selectedTile,
        bool hasActionTargetTile,
        TileCoord actionTargetTile) const
    {
        HBRUSH background = CreateSolidBrush(RGB(16, 18, 24));
        FillRect(dc, &clientRect, background);
        DeleteObject(background);

        drawTiles(dc, worldState, camera);
        drawPath(dc, worldState, camera, currentPath, pathIndex);
        drawSelectedTile(dc, worldState, camera, hasSelectedTile, selectedTile);
        drawActionTarget(dc, worldState, camera, hasActionTargetTile, actionTargetTile);
        drawAuthoringHotspots(dc, worldState, camera, hasHoveredTile, hoveredTile, hasSelectedTile, selectedTile);
        drawHoveredTile(dc, worldState, camera, hasHoveredTile, hoveredTile);
        drawEntities(dc, worldState, camera, hasHoveredTile, hoveredTile, hasSelectedTile, selectedTile);
        drawRemotePresence(dc, camera, remotePresenceProxies);
        drawPlayer(dc, worldState, camera, playerPosition);
    }

    void WorldRenderer::drawTiles(HDC dc, const WorldState& worldState, const Camera2D& camera) const
    {
        const int width = worldState.world().getWidth();
        const int height = worldState.world().getHeight();

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const TileCoord tile{ x, y };
                const RECT rect = tileToScreenRect(worldState, camera, tile);
                const bool blocked = worldState.world().isBlocked(tile);
                const BgfxWorldThemeId theme = worldState.visualThemeForTile(tile);
                const BgfxThemePaletteMode paletteMode = worldState.paletteMode();

                HBRUSH brush = CreateSolidBrush(tileFillColor(theme, paletteMode, blocked, tileVariant(tile)));
                FillRect(dc, &rect, brush);
                DeleteObject(brush);

                HPEN pen = CreatePen(PS_SOLID, 1, tileOutlineColor(theme));
                HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
                HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));

                Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);

                if (worldState.regionOverlayEnabled())
                {
                    const int tileScreenWidth = rect.right - rect.left;
                    const int boundaryThickness = tileScreenWidth / 6 > 4 ? tileScreenWidth / 6 : 4;
                    const COLORREF boundary = regionBoundaryColor(theme, paletteMode);

                    const TileCoord east{ x + 1, y };
                    if (x + 1 < width && worldState.visualThemeForTile(east) != theme)
                    {
                        RECT boundaryRect{
                            rect.right - boundaryThickness / 2,
                            rect.top,
                            rect.right + boundaryThickness / 2,
                            rect.bottom
                        };

                        HBRUSH boundaryBrush = CreateSolidBrush(boundary);
                        FillRect(dc, &boundaryRect, boundaryBrush);
                        DeleteObject(boundaryBrush);
                    }

                    const TileCoord south{ x, y + 1 };
                    if (y + 1 < height && worldState.visualThemeForTile(south) != theme)
                    {
                        RECT boundaryRect{
                            rect.left,
                            rect.bottom - boundaryThickness / 2,
                            rect.right,
                            rect.bottom + boundaryThickness / 2
                        };

                        HBRUSH boundaryBrush = CreateSolidBrush(boundary);
                        FillRect(dc, &boundaryRect, boundaryBrush);
                        DeleteObject(boundaryBrush);
                    }
                }

                SelectObject(dc, oldBrush);
                SelectObject(dc, oldPen);
                DeleteObject(pen);
            }
        }
    }

    void WorldRenderer::drawPath(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex) const
    {
        if (currentPath.empty() || pathIndex >= currentPath.size())
        {
            return;
        }

        HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 190, 112));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
        HBRUSH brush = CreateSolidBrush(RGB(255, 205, 132));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));

        for (size_t i = pathIndex; i < currentPath.size(); ++i)
        {
            const Vec2 screen = camera.worldToScreen(worldState.world().tileToWorldCenter(currentPath[i]));
            const int radius = i + 1 == currentPath.size() ? 7 : 5;

            Ellipse(dc,
                static_cast<int>(screen.x) - radius,
                static_cast<int>(screen.y) - radius,
                static_cast<int>(screen.x) + radius,
                static_cast<int>(screen.y) + radius);

            if (i > pathIndex)
            {
                const Vec2 prev = camera.worldToScreen(worldState.world().tileToWorldCenter(currentPath[i - 1]));
                MoveToEx(dc, static_cast<int>(prev.x), static_cast<int>(prev.y), nullptr);
                LineTo(dc, static_cast<int>(screen.x), static_cast<int>(screen.y));
            }
        }

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(brush);
        DeleteObject(pen);
    }

    void WorldRenderer::drawHoveredTile(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        bool hasHoveredTile,
        TileCoord hoveredTile) const
    {
        if (!hasHoveredTile || !worldState.world().isInBounds(hoveredTile))
        {
            return;
        }

        const RECT rect = tileToScreenRect(worldState, camera, hoveredTile);
        const COLORREF color = hoveredOutlineColor(worldState, hoveredTile);

        HPEN pen = CreatePen(PS_SOLID, 3, color);
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));

        Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);
        Rectangle(dc, rect.left + 4, rect.top + 4, rect.right - 4, rect.bottom - 4);

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }

    void WorldRenderer::drawSelectedTile(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        bool hasSelectedTile,
        TileCoord selectedTile) const
    {
        if (!hasSelectedTile || !worldState.world().isInBounds(selectedTile))
        {
            return;
        }

        const RECT rect = tileToScreenRect(worldState, camera, selectedTile);
        HPEN pen = CreatePen(PS_DOT, 2, RGB(132, 188, 255));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));

        Rectangle(dc, rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2);

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }

    void WorldRenderer::drawActionTarget(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        bool hasActionTargetTile,
        TileCoord actionTargetTile) const
    {
        if (!hasActionTargetTile || !worldState.world().isInBounds(actionTargetTile))
        {
            return;
        }

        const RECT rect = tileToScreenRect(worldState, camera, actionTargetTile);
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 214, 118));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));

        Rectangle(dc, rect.left + 6, rect.top + 6, rect.right - 6, rect.bottom - 6);
        MoveToEx(dc, rect.left + 8, (rect.top + rect.bottom) / 2, nullptr);
        LineTo(dc, rect.right - 8, (rect.top + rect.bottom) / 2);
        MoveToEx(dc, (rect.left + rect.right) / 2, rect.top + 8, nullptr);
        LineTo(dc, (rect.left + rect.right) / 2, rect.bottom - 8);

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }

    void WorldRenderer::drawAuthoringHotspots(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        bool hasHoveredTile,
        TileCoord hoveredTile,
        bool hasSelectedTile,
        TileCoord selectedTile) const
    {
        if (!worldState.authoringHotspotsVisible())
        {
            return;
        }

        for (const WorldAuthoringHotspot& hotspot : worldState.authoringHotspots())
        {
            const RECT rect = tileToScreenRect(worldState, camera, hotspot.tile);
            const COLORREF color = hotspotMarkerColor(hotspot);
            const bool emphasized =
                (hasHoveredTile && hotspot.tile == hoveredTile) ||
                (hasSelectedTile && hotspot.tile == selectedTile);

            HPEN pen = CreatePen(PS_SOLID, emphasized ? 3 : 2, color);
            HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
            HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));

            Ellipse(dc, rect.left + 8, rect.top + 8, rect.right - 8, rect.bottom - 8);

            SelectObject(dc, oldBrush);
            SelectObject(dc, oldPen);
            DeleteObject(pen);

            HBRUSH centerBrush = CreateSolidBrush(color);
            const int centerX = (rect.left + rect.right) / 2;
            const int centerY = (rect.top + rect.bottom) / 2;
            RECT markerRect{ centerX - 4, centerY - 4, centerX + 4, centerY + 4 };
            FillRect(dc, &markerRect, centerBrush);
            DeleteObject(centerBrush);
        }
    }

    void WorldRenderer::drawEntities(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        bool hasHoveredTile,
        TileCoord hoveredTile,
        bool hasSelectedTile,
        TileCoord selectedTile) const
    {
        for (const Entity& entity : worldState.entities().all())
        {
            HBRUSH brush = CreateSolidBrush(entityFillColor(worldState, entity));
            HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));
            HPEN pen = CreatePen(PS_SOLID, 1, entityOutlineColor(worldState, entity));
            HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));

            const Vec2 screen = camera.worldToScreen(worldState.world().tileToWorldCenter(entity.tile));
            const bool emphasized =
                (hasHoveredTile && entity.tile == hoveredTile) ||
                (hasSelectedTile && entity.tile == selectedTile);
            const int halfSize = emphasized
                ? static_cast<int>(10.0f * camera.getZoom())
                : static_cast<int>(8.0f * camera.getZoom());

            Rectangle(dc,
                static_cast<int>(screen.x) - halfSize,
                static_cast<int>(screen.y) - halfSize,
                static_cast<int>(screen.x) + halfSize,
                static_cast<int>(screen.y) + halfSize);

            RECT accentRect{
                static_cast<LONG>(screen.x) + halfSize - 6,
                static_cast<LONG>(screen.y) - halfSize,
                static_cast<LONG>(screen.x) + halfSize,
                static_cast<LONG>(screen.y) - halfSize + 6
            };
            HBRUSH accentBrush = CreateSolidBrush(entityStateAccentColor(entity));
            FillRect(dc, &accentRect, accentBrush);
            DeleteObject(accentBrush);

            if (emphasized)
            {
                HPEN emphasisPen = CreatePen(PS_SOLID, 2, RGB(240, 240, 255));
                HPEN previousPen = static_cast<HPEN>(SelectObject(dc, emphasisPen));
                HBRUSH emphasisBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));
                Rectangle(dc,
                    static_cast<int>(screen.x) - halfSize - 3,
                    static_cast<int>(screen.y) - halfSize - 3,
                    static_cast<int>(screen.x) + halfSize + 3,
                    static_cast<int>(screen.y) + halfSize + 3);
                SelectObject(dc, emphasisBrush);
                SelectObject(dc, previousPen);
                DeleteObject(emphasisPen);
            }

            SelectObject(dc, oldBrush);
            SelectObject(dc, oldPen);
            DeleteObject(brush);
            DeleteObject(pen);
        }
    }

    void WorldRenderer::drawRemotePresence(
        HDC dc,
        const Camera2D& camera,
        const std::vector<RemotePresenceRenderProxy>& remotePresenceProxies) const
    {
        HBRUSH brush = CreateSolidBrush(RGB(120, 232, 210));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(220, 255, 246));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));

        for (const RemotePresenceRenderProxy& proxy : remotePresenceProxies)
        {
            const Vec2 screen = camera.worldToScreen(proxy.worldPosition);
            const int radius = static_cast<int>(8.0f * camera.getZoom());

            Ellipse(dc,
                static_cast<int>(screen.x) - radius,
                static_cast<int>(screen.y) - radius,
                static_cast<int>(screen.x) + radius,
                static_cast<int>(screen.y) + radius);
        }

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(brush);
        DeleteObject(pen);
    }

    void WorldRenderer::drawPlayer(HDC dc, const WorldState& worldState, const Camera2D& camera, const Vec2& playerPosition) const
    {
        const Vec2 screen = camera.worldToScreen(playerPosition);
        const int radius = static_cast<int>(12.0f * camera.getZoom());

        const TileCoord playerTile = worldState.world().worldToTile(playerPosition);
        const RECT playerTileRect = tileToScreenRect(worldState, camera, playerTile);
        HPEN tilePen = CreatePen(PS_DOT, 1, RGB(96, 166, 220));
        HPEN oldTilePen = static_cast<HPEN>(SelectObject(dc, tilePen));
        HBRUSH oldTileBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));
        Rectangle(dc, playerTileRect.left + 10, playerTileRect.top + 10, playerTileRect.right - 10, playerTileRect.bottom - 10);
        SelectObject(dc, oldTileBrush);
        SelectObject(dc, oldTilePen);
        DeleteObject(tilePen);

        HBRUSH brush = CreateSolidBrush(RGB(160, 210, 255));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(220, 240, 255));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));

        Ellipse(dc,
            static_cast<int>(screen.x) - radius,
            static_cast<int>(screen.y) - radius,
            static_cast<int>(screen.x) + radius,
            static_cast<int>(screen.y) + radius);

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(brush);
        DeleteObject(pen);
    }

    RECT WorldRenderer::tileToScreenRect(
        const WorldState& worldState,
        const Camera2D& camera,
        TileCoord tile) const
    {
        const int tileSize = worldState.world().getTileSize();

        const Vec2 center = worldState.world().tileToWorldCenter(tile);
        const Vec2 topLeftWorld{
            center.x - static_cast<float>(tileSize) * 0.5f,
            center.y - static_cast<float>(tileSize) * 0.5f
        };
        const Vec2 bottomRightWorld{
            center.x + static_cast<float>(tileSize) * 0.5f,
            center.y + static_cast<float>(tileSize) * 0.5f
        };

        const Vec2 topLeft = camera.worldToScreen(topLeftWorld);
        const Vec2 bottomRight = camera.worldToScreen(bottomRightWorld);

        return RECT{
            static_cast<LONG>(topLeft.x),
            static_cast<LONG>(topLeft.y),
            static_cast<LONG>(bottomRight.x),
            static_cast<LONG>(bottomRight.y)
        };
    }
}
