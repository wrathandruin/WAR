#include "engine/render/WorldRenderer.h"

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
        for (int y = 0; y < worldState.world().getHeight(); ++y)
        {
            for (int x = 0; x < worldState.world().getWidth(); ++x)
            {
                const TileCoord tile{ x, y };
                const RECT rect = tileToScreenRect(worldState, camera, tile);
                const bool blocked = worldState.world().isBlocked(tile);

                HBRUSH brush = CreateSolidBrush(
                    blocked ? RGB(220, 60, 60) : RGB(34, 38, 46));
                FillRect(dc, &rect, brush);
                DeleteObject(brush);

                HPEN pen = CreatePen(
                    PS_SOLID,
                    1,
                    blocked ? RGB(255, 180, 180) : RGB(45, 50, 60));
                HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
                HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));

                Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);

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
