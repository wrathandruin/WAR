#include "engine/render/DebugOverlayRenderer.h"

#include <cstdio>
#include <cstring>
#include <string>

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

        const char* hotspotStateText(const WorldAuthoringHotspot& hotspot)
        {
            return hotspot.encounterReady ? "encounter-ready" : "staged";
        }

        std::string tileText(bool hasTile, TileCoord tile)
        {
            if (!hasTile)
            {
                return "none";
            }

            return "(" + std::to_string(tile.x) + ", " + std::to_string(tile.y) + ")";
        }

        std::string hoverAffordance(
            const WorldState& worldState,
            bool hasHoveredTile,
            TileCoord hoveredTile,
            const Entity* hoveredEntity,
            const WorldAuthoringHotspot* hoveredHotspot)
        {
            if (!hasHoveredTile || !worldState.world().isInBounds(hoveredTile))
            {
                return "none";
            }

            if (worldState.world().isBlocked(hoveredTile))
            {
                return "blocked terrain";
            }

            if (hoveredEntity != nullptr)
            {
                return std::string("interactable ") + hoveredEntity->name;
            }

            if (hoveredHotspot != nullptr)
            {
                return std::string("authored hotspot ") + hoveredHotspot->label;
            }

            return "walkable terrain";
        }

        std::string hoverPrompt(
            const WorldState& worldState,
            bool hasHoveredTile,
            TileCoord hoveredTile,
            const Entity* hoveredEntity,
            const WorldAuthoringHotspot* hoveredHotspot)
        {
            if (!hasHoveredTile || !worldState.world().isInBounds(hoveredTile))
            {
                return "Move the cursor onto the world to preview actions.";
            }

            if (worldState.world().isBlocked(hoveredTile))
            {
                return "Blocked tile. Shift+RMB inspect for detail.";
            }

            if (hoveredEntity != nullptr)
            {
                return std::string("RMB interact with ") + hoveredEntity->name + " | Shift+RMB inspect";
            }

            if (hoveredHotspot != nullptr)
            {
                return std::string("RMB use ") + hoveredHotspot->label + " | Shift+RMB inspect hotspot";
            }

            return "LMB move | Shift+RMB inspect terrain";
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

    const char* DebugOverlayRenderer::hotspotTypeToText(WorldAuthoringHotspotType type) const
    {
        switch (type)
        {
        case WorldAuthoringHotspotType::Encounter:
            return "encounter";
        case WorldAuthoringHotspotType::Control:
            return "control";
        case WorldAuthoringHotspotType::Transit:
            return "transit";
        case WorldAuthoringHotspotType::Loot:
            return "loot";
        case WorldAuthoringHotspotType::Hazard:
            return "hazard";
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
        bool hasSelectedTile,
        TileCoord selectedTile,
        bool hasActionTargetTile,
        TileCoord actionTargetTile,
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
        const WorldAuthoringHotspot* hoveredHotspot =
            hasHoveredTile && worldState.world().isInBounds(hoveredTile)
                ? worldState.authoringHotspotAt(hoveredTile)
                : nullptr;

        const std::string affordance = hoverAffordance(worldState, hasHoveredTile, hoveredTile, hoveredEntity, hoveredHotspot);
        const std::string prompt = hoverPrompt(worldState, hasHoveredTile, hoveredTile, hoveredEntity, hoveredHotspot);
        const std::string selected = tileText(hasSelectedTile, selectedTile);
        const std::string actionTarget = tileText(hasActionTargetTile, actionTargetTile);
        const std::string pathDestination =
            currentPath.empty() || pathIndex >= currentPath.size()
                ? "none"
                : tileText(true, currentPath.back());

        char buffer[2048]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "WAR Milestone 30\n"
            "LMB: move    RMB: interact    Shift+RMB: inspect    MMB drag: pan    Wheel: zoom\n"
            "Authoring: O region overlay    H hotspot overlay    7/8/9 palette\n"
            "Player world: (%.1f, %.1f)\n"
            "Player tile: (%d, %d)\n"
            "Mouse tile: (%d, %d)\n"
            "Hovered region: %s\n"
            "Hovered blocked: %s\n"
            "Hovered affordance: %s\n"
            "Prompt: %s\n"
            "Hovered entity: %s\n"
            "Hovered entity type: %s\n"
            "Hovered entity state: %s\n"
            "Hovered hotspot: %s\n"
            "Hovered hotspot type: %s\n"
            "Hovered hotspot state: %s\n"
            "Hovered hotspot summary: %s\n"
            "Selected tile: %s\n"
            "Move target: %s\n"
            "Path destination: %s\n"
            "Region overlay: %s\n"
            "Hotspot overlay: %s\n"
            "Camera: (%.1f, %.1f)  Zoom: %.2f\n"
            "Path nodes remaining: %zu\n"
            "Entities: %zu\n"
            "Hotspots: %zu\n"
            "Frame dt: %.4f",
            playerPosition.x,
            playerPosition.y,
            playerTile.x,
            playerTile.y,
            mouseTile.x,
            mouseTile.y,
            hasHoveredTile && worldState.world().isInBounds(hoveredTile)
                ? WorldRegionTags::debugName(worldState.regionTag(hoveredTile))
                : "none",
            hoveredBlocked ? "yes" : "no",
            affordance.c_str(),
            prompt.c_str(),
            hoveredEntity ? hoveredEntity->name.c_str() : "none",
            hoveredEntity ? entityTypeToText(hoveredEntity->type) : "none",
            hoveredEntity ? entityStateText(*hoveredEntity) : "none",
            hoveredHotspot ? hoveredHotspot->label.c_str() : "none",
            hoveredHotspot ? hotspotTypeToText(hoveredHotspot->type) : "none",
            hoveredHotspot ? hotspotStateText(*hoveredHotspot) : "none",
            hoveredHotspot ? hoveredHotspot->summary.c_str() : "none",
            selected.c_str(),
            actionTarget.c_str(),
            pathDestination.c_str(),
            worldState.regionOverlayEnabled() ? "on" : "off",
            worldState.authoringHotspotsVisible() ? "on" : "off",
            camera.getPosition().x,
            camera.getPosition().y,
            camera.getZoom(),
            pathIndex < currentPath.size() ? currentPath.size() - pathIndex : 0,
            worldState.entities().count(),
            worldState.authoringHotspots().size(),
            lastDeltaTime);

        TextOutA(dc, 16, 16, buffer, static_cast<int>(std::strlen(buffer)));

        int y = 392;
        TextOutA(dc, 16, y, "Event Log:", 10);
        y += 22;

        for (const std::string& entry : eventLog)
        {
            TextOutA(dc, 16, y, entry.c_str(), static_cast<int>(entry.size()));
            y += 18;
        }
    }
}
