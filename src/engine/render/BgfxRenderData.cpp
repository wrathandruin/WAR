#include "engine/render/BgfxRenderData.h"

#include "engine/gameplay/Entity.h"
#include "engine/render/BgfxTileVisuals.h"
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

        BgfxQuad tileToWorldQuad(const WorldState& worldState, TileCoord tile, uint32_t color)
        {
            const int tileSize = worldState.world().getTileSize();
            const Vec2 center = worldState.world().tileToWorldCenter(tile);

            return BgfxQuad{
                center.x - static_cast<float>(tileSize) * 0.5f,
                center.y - static_cast<float>(tileSize) * 0.5f,
                center.x + static_cast<float>(tileSize) * 0.5f,
                center.y + static_cast<float>(tileSize) * 0.5f,
                color
            };
        }

        BgfxTexturedQuad tileToWorldTexturedQuad(
            const WorldState& worldState,
            TileCoord tile,
            uint32_t color,
            BgfxSpriteMaterialId material)
        {
            const int tileSize = worldState.world().getTileSize();
            const Vec2 center = worldState.world().tileToWorldCenter(tile);

            return BgfxTexturedQuad{
                center.x - static_cast<float>(tileSize) * 0.5f,
                center.y - static_cast<float>(tileSize) * 0.5f,
                center.x + static_cast<float>(tileSize) * 0.5f,
                center.y + static_cast<float>(tileSize) * 0.5f,
                color,
                material
            };
        }

        BgfxQuad centeredWorldQuad(const Vec2& worldPosition, float halfSize, uint32_t color)
        {
            return BgfxQuad{
                worldPosition.x - halfSize,
                worldPosition.y - halfSize,
                worldPosition.x + halfSize,
                worldPosition.y + halfSize,
                color
            };
        }

        BgfxTexturedQuad centeredWorldTexturedQuad(
            const Vec2& worldPosition,
            float halfSize,
            uint32_t color,
            BgfxSpriteMaterialId material)
        {
            return BgfxTexturedQuad{
                worldPosition.x - halfSize,
                worldPosition.y - halfSize,
                worldPosition.x + halfSize,
                worldPosition.y + halfSize,
                color,
                material
            };
        }

        BgfxQuad rightBoundaryQuad(const WorldState& worldState, TileCoord tile, float thickness, uint32_t color)
        {
            const int tileSize = worldState.world().getTileSize();
            const Vec2 center = worldState.world().tileToWorldCenter(tile);
            const float halfTile = static_cast<float>(tileSize) * 0.5f;
            const float halfThickness = thickness * 0.5f;

            return BgfxQuad{
                center.x + halfTile - halfThickness,
                center.y - halfTile,
                center.x + halfTile + halfThickness,
                center.y + halfTile,
                color
            };
        }

        BgfxQuad bottomBoundaryQuad(const WorldState& worldState, TileCoord tile, float thickness, uint32_t color)
        {
            const int tileSize = worldState.world().getTileSize();
            const Vec2 center = worldState.world().tileToWorldCenter(tile);
            const float halfTile = static_cast<float>(tileSize) * 0.5f;
            const float halfThickness = thickness * 0.5f;

            return BgfxQuad{
                center.x - halfTile,
                center.y + halfTile - halfThickness,
                center.x + halfTile,
                center.y + halfTile + halfThickness,
                color
            };
        }

        uint32_t pathColor()
        {
            return rgbaToAbgr(255, 192, 116, 220);
        }

        uint32_t pathDestinationColor()
        {
            return rgbaToAbgr(255, 220, 142, 240);
        }

        uint32_t playerColor()
        {
            return rgbaToAbgr(180, 225, 255);
        }

        uint32_t selectedTileColor()
        {
            return rgbaToAbgr(132, 188, 255, 92);
        }

        uint32_t selectedTileCenterColor()
        {
            return rgbaToAbgr(188, 224, 255, 170);
        }

        uint32_t actionTargetColor()
        {
            return rgbaToAbgr(255, 214, 118, 104);
        }

        uint32_t actionTargetCenterColor()
        {
            return rgbaToAbgr(255, 232, 168, 188);
        }

        bool tileHasInteraction(const WorldState& worldState, TileCoord tile)
        {
            return worldState.entities().getAt(tile) != nullptr || worldState.authoringHotspotAt(tile) != nullptr;
        }

        uint32_t hoveredColor(const WorldState& worldState, TileCoord tile)
        {
            if (worldState.world().isBlocked(tile))
            {
                return rgbaToAbgr(255, 140, 140, 140);
            }

            if (tileHasInteraction(worldState, tile))
            {
                return rgbaToAbgr(132, 228, 255, 150);
            }

            return rgbaToAbgr(240, 230, 100, 128);
        }

        uint32_t entityColor(const WorldState& worldState, const Entity& entity)
        {
            switch (worldState.regionTag(entity.tile))
            {
            case WorldRegionTagId::CargoBay:
                switch (entity.type)
                {
                case EntityType::Crate:
                    return entity.isLocked
                        ? rgbaToAbgr(176, 120, 78)
                        : rgbaToAbgr(212, 168, 104);

                case EntityType::Terminal:
                    return entity.isPowered
                        ? rgbaToAbgr(120, 176, 212)
                        : rgbaToAbgr(96, 124, 148);

                case EntityType::Locker:
                    return entity.isLocked
                        ? rgbaToAbgr(188, 150, 118)
                        : rgbaToAbgr(166, 166, 176);

                default:
                    break;
                }
                break;

            case WorldRegionTagId::TransitSpine:
                switch (entity.type)
                {
                case EntityType::Crate:
                    return rgbaToAbgr(192, 176, 128);

                case EntityType::Terminal:
                    return entity.isPowered
                        ? rgbaToAbgr(146, 214, 244)
                        : rgbaToAbgr(114, 152, 180);

                case EntityType::Locker:
                    return entity.isLocked
                        ? rgbaToAbgr(212, 186, 124)
                        : rgbaToAbgr(186, 190, 206);

                default:
                    break;
                }
                break;

            case WorldRegionTagId::MedLab:
                switch (entity.type)
                {
                case EntityType::Crate:
                    return rgbaToAbgr(214, 224, 232);

                case EntityType::Terminal:
                    return entity.isPowered
                        ? rgbaToAbgr(134, 236, 255)
                        : rgbaToAbgr(118, 170, 188);

                case EntityType::Locker:
                    return entity.isLocked
                        ? rgbaToAbgr(210, 228, 240)
                        : rgbaToAbgr(234, 242, 252);

                default:
                    break;
                }
                break;

            case WorldRegionTagId::CommandDeck:
                switch (entity.type)
                {
                case EntityType::Crate:
                    return rgbaToAbgr(188, 198, 216);

                case EntityType::Terminal:
                    return entity.isPowered
                        ? rgbaToAbgr(120, 188, 255)
                        : rgbaToAbgr(92, 132, 186);

                case EntityType::Locker:
                    return entity.isLocked
                        ? rgbaToAbgr(220, 210, 166)
                        : rgbaToAbgr(212, 220, 236);

                default:
                    break;
                }
                break;

            case WorldRegionTagId::HazardContainment:
                switch (entity.type)
                {
                case EntityType::Crate:
                    return entity.isLocked
                        ? rgbaToAbgr(184, 96, 96)
                        : rgbaToAbgr(158, 84, 84);

                case EntityType::Terminal:
                    return entity.isPowered
                        ? rgbaToAbgr(255, 152, 132)
                        : rgbaToAbgr(164, 98, 98);

                case EntityType::Locker:
                    return entity.isLocked
                        ? rgbaToAbgr(232, 118, 118)
                        : rgbaToAbgr(202, 116, 116);

                default:
                    break;
                }
                break;

            default:
                break;
            }

            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen
                    ? rgbaToAbgr(180, 180, 180)
                    : rgbaToAbgr(215, 195, 130);

            case EntityType::Terminal:
                return entity.isPowered
                    ? rgbaToAbgr(130, 230, 255)
                    : rgbaToAbgr(120, 145, 170);

            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return rgbaToAbgr(230, 120, 120);
                }

                return entity.isOpen
                    ? rgbaToAbgr(215, 215, 240)
                    : rgbaToAbgr(190, 190, 220);

            default:
                return rgbaToAbgr(255, 255, 255);
            }
        }

        BgfxSpriteMaterialId entityMaterial(const Entity& entity)
        {
            switch (entity.type)
            {
            case EntityType::Crate:
                return BgfxSpriteMaterialId::Crate;
            case EntityType::Terminal:
                return BgfxSpriteMaterialId::Terminal;
            case EntityType::Locker:
                return BgfxSpriteMaterialId::Locker;
            default:
                return BgfxSpriteMaterialId::Crate;
            }
        }

        uint32_t hotspotTileColor(const WorldAuthoringHotspot& hotspot)
        {
            switch (hotspot.type)
            {
            case WorldAuthoringHotspotType::Encounter:
                return hotspot.encounterReady
                    ? rgbaToAbgr(255, 204, 100, 68)
                    : rgbaToAbgr(214, 176, 92, 56);

            case WorldAuthoringHotspotType::Control:
                return hotspot.encounterReady
                    ? rgbaToAbgr(120, 188, 255, 72)
                    : rgbaToAbgr(96, 148, 212, 56);

            case WorldAuthoringHotspotType::Transit:
                return hotspot.encounterReady
                    ? rgbaToAbgr(120, 236, 255, 72)
                    : rgbaToAbgr(108, 202, 220, 56);

            case WorldAuthoringHotspotType::Loot:
                return hotspot.encounterReady
                    ? rgbaToAbgr(238, 222, 138, 72)
                    : rgbaToAbgr(210, 194, 120, 56);

            case WorldAuthoringHotspotType::Hazard:
                return hotspot.encounterReady
                    ? rgbaToAbgr(255, 136, 136, 76)
                    : rgbaToAbgr(212, 118, 118, 60);

            default:
                return rgbaToAbgr(240, 240, 240, 56);
            }
        }

        uint32_t hotspotCenterColor(const WorldAuthoringHotspot& hotspot)
        {
            switch (hotspot.type)
            {
            case WorldAuthoringHotspotType::Encounter:
                return hotspot.encounterReady
                    ? rgbaToAbgr(255, 214, 112)
                    : rgbaToAbgr(220, 186, 98);

            case WorldAuthoringHotspotType::Control:
                return hotspot.encounterReady
                    ? rgbaToAbgr(128, 198, 255)
                    : rgbaToAbgr(102, 160, 220);

            case WorldAuthoringHotspotType::Transit:
                return hotspot.encounterReady
                    ? rgbaToAbgr(130, 240, 255)
                    : rgbaToAbgr(112, 208, 226);

            case WorldAuthoringHotspotType::Loot:
                return hotspot.encounterReady
                    ? rgbaToAbgr(242, 228, 150)
                    : rgbaToAbgr(216, 198, 126);

            case WorldAuthoringHotspotType::Hazard:
                return hotspot.encounterReady
                    ? rgbaToAbgr(255, 146, 146)
                    : rgbaToAbgr(220, 124, 124);

            default:
                return rgbaToAbgr(255, 255, 255);
            }
        }
    }

    BgfxWorldRenderData BgfxRenderDataBuilder::build(
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
        TileCoord actionTargetTile)
    {
        (void)camera;

        BgfxWorldRenderData data{};

        const int width = worldState.world().getWidth();
        const int height = worldState.world().getHeight();

        data.tiles.quads.reserve(static_cast<size_t>(width) * static_cast<size_t>(height));

        if (worldState.regionOverlayEnabled())
        {
            data.regionOverlay.quads.reserve(static_cast<size_t>(width) * static_cast<size_t>(height));
        }

        if (worldState.authoringHotspotsVisible())
        {
            data.authoringHotspots.quads.reserve(worldState.authoringHotspots().size() * 4u);
        }

        const float boundaryThickness = 8.0f;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const TileCoord tile{ x, y };

                data.tiles.quads.push_back(
                    tileToWorldTexturedQuad(
                        worldState,
                        tile,
                        BgfxTileVisuals::tintForTile(worldState, tile),
                        BgfxTileVisuals::materialForTile(worldState, tile)));

                if (!worldState.regionOverlayEnabled())
                {
                    continue;
                }

                const BgfxWorldThemeId theme = worldState.visualThemeForTile(tile);
                const BgfxThemePaletteMode paletteMode = worldState.paletteMode();
                const uint32_t boundaryColor = BgfxWorldTheme::regionBoundaryColor(theme, paletteMode);

                const TileCoord east{ x + 1, y };
                if (x + 1 < width && worldState.visualThemeForTile(east) != theme)
                {
                    data.regionOverlay.quads.push_back(
                        rightBoundaryQuad(worldState, tile, boundaryThickness, boundaryColor));
                }

                const TileCoord south{ x, y + 1 };
                if (y + 1 < height && worldState.visualThemeForTile(south) != theme)
                {
                    data.regionOverlay.quads.push_back(
                        bottomBoundaryQuad(worldState, tile, boundaryThickness, boundaryColor));
                }
            }
        }

        if (pathIndex < currentPath.size())
        {
            data.path.quads.reserve((currentPath.size() - pathIndex) + 1u);
        }

        for (size_t i = pathIndex; i < currentPath.size(); ++i)
        {
            const Vec2 center = worldState.world().tileToWorldCenter(currentPath[i]);
            data.path.quads.push_back(
                centeredWorldQuad(
                    center,
                    i + 1 == currentPath.size() ? 6.5f : 5.0f,
                    i + 1 == currentPath.size() ? pathDestinationColor() : pathColor()));
        }

        if (hasHoveredTile && worldState.world().isInBounds(hoveredTile))
        {
            data.hoveredTile.quads.push_back(
                tileToWorldQuad(worldState, hoveredTile, hoveredColor(worldState, hoveredTile)));
            if (tileHasInteraction(worldState, hoveredTile))
            {
                data.hoveredTile.quads.push_back(
                    centeredWorldQuad(worldState.world().tileToWorldCenter(hoveredTile), 11.0f, rgbaToAbgr(255, 255, 255, 70)));
            }
        }

        if (hasSelectedTile && worldState.world().isInBounds(selectedTile))
        {
            data.selectedTile.quads.push_back(
                tileToWorldQuad(worldState, selectedTile, selectedTileColor()));
            data.selectedTile.quads.push_back(
                centeredWorldQuad(worldState.world().tileToWorldCenter(selectedTile), 10.0f, selectedTileCenterColor()));
        }

        if (hasActionTargetTile && worldState.world().isInBounds(actionTargetTile))
        {
            data.actionTarget.quads.push_back(
                tileToWorldQuad(worldState, actionTargetTile, actionTargetColor()));
            data.actionTarget.quads.push_back(
                centeredWorldQuad(worldState.world().tileToWorldCenter(actionTargetTile), 12.0f, actionTargetCenterColor()));
        }

        if (worldState.authoringHotspotsVisible())
        {
            for (const WorldAuthoringHotspot& hotspot : worldState.authoringHotspots())
            {
                const Vec2 center = worldState.world().tileToWorldCenter(hotspot.tile);
                const bool emphasized =
                    (hasHoveredTile && hotspot.tile == hoveredTile) ||
                    (hasSelectedTile && hotspot.tile == selectedTile);

                data.authoringHotspots.quads.push_back(
                    tileToWorldQuad(worldState, hotspot.tile, hotspotTileColor(hotspot)));
                data.authoringHotspots.quads.push_back(
                    centeredWorldQuad(center, hotspot.encounterReady ? 10.0f : 8.0f, hotspotCenterColor(hotspot)));

                if (hotspot.encounterReady)
                {
                    data.authoringHotspots.quads.push_back(
                        centeredWorldQuad(center, 16.0f, rgbaToAbgr(255, 255, 255, 48)));
                }

                if (emphasized)
                {
                    data.authoringHotspots.quads.push_back(
                        centeredWorldQuad(center, 20.0f, rgbaToAbgr(255, 255, 255, 44)));
                }
            }
        }

        data.actors.quads.reserve(worldState.entities().all().size() + 1u);

        for (const Entity& entity : worldState.entities().all())
        {
            const bool emphasized =
                (hasHoveredTile && entity.tile == hoveredTile) ||
                (hasSelectedTile && entity.tile == selectedTile);

            data.actors.quads.push_back(
                centeredWorldTexturedQuad(
                    worldState.world().tileToWorldCenter(entity.tile),
                    emphasized ? 13.5f : 12.0f,
                    entityColor(worldState, entity),
                    entityMaterial(entity)));
        }

        data.actors.quads.push_back(
            centeredWorldTexturedQuad(
                playerPosition,
                14.0f,
                playerColor(),
                BgfxSpriteMaterialId::Player));

        return data;
    }
}
