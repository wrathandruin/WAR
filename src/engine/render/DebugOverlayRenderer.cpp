#include "engine/render/DebugOverlayRenderer.h"

#include <cstdio>
#include <cstring>

namespace war
{
    namespace
    {
        const char* entityStateText(const Entity& entity)
        {
            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen ? "open" : "closed";

            case EntityType::Terminal:
                return entity.isPowered ? "powered" : "offline";

            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return "locked";
                }
                return entity.isOpen ? "open" : "closed";

            default:
                return "unknown";
            }
        }
    }

    const char* DebugOverlayRenderer::entityTypeToText(EntityType type) const
    {
        switch (type)
        {
        case EntityType::Crate:
            return "crate";
        case EntityType::Terminal:
            return "terminal";
        case EntityType::Locker:
            return "locker";
        default:
            return "unknown";
        }
    }

    void DebugOverlayRenderer::render(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        const Vec2& playerPosition,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex,
        bool hasHoveredTile,
        TileCoord hoveredTile,
        const std::vector<std::string>& eventLog,
        float lastDeltaTime,
        const POINT& mousePosition) const
    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(225, 225, 225));

        const Vec2 mouseWorld = camera.screenToWorld(mousePosition.x, mousePosition.y);
        const TileCoord mouseTile = worldState.world().worldToTile(mouseWorld);
        const TileCoord playerTile = worldState.world().worldToTile(playerPosition);
        const bool hoveredBlocked =
            hasHoveredTile && worldState.world().isInBounds(hoveredTile) && worldState.world().isBlocked(hoveredTile);
        const Entity* hoveredEntity =
            hasHoveredTile && worldState.world().isInBounds(hoveredTile)
                ? worldState.entities().getAt(hoveredTile)
                : nullptr;

        char buffer[1024]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "WAR Milestone 8\n"
            "LMB: move    RMB: interact    Shift+RMB: inspect    MMB drag: pan    Wheel: zoom\n"
            "Player world: (%.1f, %.1f)\n"
            "Player tile: (%d, %d)\n"
            "Mouse tile: (%d, %d)\n"
            "Hovered blocked: %s\n"
            "Hovered entity: %s\n"
            "Hovered entity type: %s\n"
            "Hovered entity state: %s\n"
            "Camera: (%.1f, %.1f)  Zoom: %.2f\n"
            "Path nodes remaining: %zu\n"
            "Entities: %zu\n"
            "Frame dt: %.4f",
            playerPosition.x,
            playerPosition.y,
            playerTile.x,
            playerTile.y,
            mouseTile.x,
            mouseTile.y,
            hoveredBlocked ? "yes" : "no",
            hoveredEntity ? hoveredEntity->name.c_str() : "none",
            hoveredEntity ? entityTypeToText(hoveredEntity->type) : "none",
            hoveredEntity ? entityStateText(*hoveredEntity) : "none",
            camera.getPosition().x,
            camera.getPosition().y,
            camera.getZoom(),
            pathIndex < currentPath.size() ? currentPath.size() - pathIndex : 0,
            worldState.entities().count(),
            lastDeltaTime);

        TextOutA(dc, 16, 16, buffer, static_cast<int>(std::strlen(buffer)));

        int y = 240;
        TextOutA(dc, 16, y, "Event Log:", 10);
        y += 22;

        for (const std::string& entry : eventLog)
        {
            TextOutA(dc, 16, y, entry.c_str(), static_cast<int>(entry.size()));
            y += 18;
        }
    }
}
