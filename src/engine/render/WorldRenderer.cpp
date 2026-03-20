#include "engine/render/WorldRenderer.h"

#include "engine/render/BgfxWorldTheme.h"

namespace war
{
    namespace
    {
        COLORREF entityFillColor(const Entity& entity)
        {
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

        COLORREF entityOutlineColor(const Entity& entity)
        {
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
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex,
        bool hasHoveredTile,
        TileCoord hoveredTile) const
    {
        HBRUSH background = CreateSolidBrush(RGB(16, 18, 24));
        FillRect(dc, &clientRect, background);
        DeleteObject(background);

        drawTiles(dc, worldState, camera);
        drawPath(dc, worldState, camera, currentPath, pathIndex);
        drawHoveredTile(dc, worldState, camera, hasHoveredTile, hoveredTile);
        drawEntities(dc, worldState, camera);
        drawPlayer(dc, camera, playerPosition);
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
                    const int boundaryThickness = max(4, (rect.right - rect.left) / 6);
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
        HPEN pen = CreatePen(
            PS_SOLID,
            2,
            worldState.world().isBlocked(hoveredTile) ? RGB(220, 100, 100) : RGB(230, 220, 110));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));

        Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }

    void WorldRenderer::drawPath(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex) const
    {
        if (currentPath.empty())
        {
            return;
        }

        HBRUSH brush = CreateSolidBrush(RGB(255, 180, 90));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 210, 130));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));

        for (size_t i = pathIndex; i < currentPath.size(); ++i)
        {
            const Vec2 screen = camera.worldToScreen(worldState.world().tileToWorldCenter(currentPath[i]));
            const int radius = static_cast<int>(5.0f * camera.getZoom());
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

    void WorldRenderer::drawEntities(HDC dc, const WorldState& worldState, const Camera2D& camera) const
    {
        for (const Entity& entity : worldState.entities().all())
        {
            HBRUSH brush = CreateSolidBrush(entityFillColor(entity));
            HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));
            HPEN pen = CreatePen(PS_SOLID, 1, entityOutlineColor(entity));
            HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));

            const Vec2 screen = camera.worldToScreen(worldState.world().tileToWorldCenter(entity.tile));
            const int halfSize = static_cast<int>(8.0f * camera.getZoom());

            Rectangle(dc,
                static_cast<int>(screen.x) - halfSize,
                static_cast<int>(screen.y) - halfSize,
                static_cast<int>(screen.x) + halfSize,
                static_cast<int>(screen.y) + halfSize);

            SelectObject(dc, oldBrush);
            SelectObject(dc, oldPen);
            DeleteObject(brush);
            DeleteObject(pen);
        }
    }

    void WorldRenderer::drawPlayer(HDC dc, const Camera2D& camera, const Vec2& playerPosition) const
    {
        const Vec2 screen = camera.worldToScreen(playerPosition);
        const int radius = static_cast<int>(12.0f * camera.getZoom());

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
