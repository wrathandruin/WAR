#include "engine/render/BgfxRenderData.h"

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

        RECT tileToScreenRect(const WorldState& worldState, const Camera2D& camera, TileCoord tile)
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

        RECT centeredScreenRect(const Camera2D& camera, const Vec2& worldPosition, float halfSize)
        {
            const Vec2 screen = camera.worldToScreen(worldPosition);

            return RECT{
                static_cast<LONG>(screen.x - halfSize),
                static_cast<LONG>(screen.y - halfSize),
                static_cast<LONG>(screen.x + halfSize),
                static_cast<LONG>(screen.y + halfSize)
            };
        }

        uint32_t tileColor(bool blocked)
        {
            return blocked
                ? rgbaToAbgr(220, 60, 60)
                : rgbaToAbgr(34, 38, 46);
        }

        uint32_t pathColor()
        {
            return rgbaToAbgr(255, 180, 90);
        }

        uint32_t playerColor()
        {
            return rgbaToAbgr(160, 210, 255);
        }

        uint32_t hoveredColor(bool blocked)
        {
            return blocked
                ? rgbaToAbgr(255, 140, 140, 140)
                : rgbaToAbgr(240, 230, 100, 140);
        }

        uint32_t entityColor(const Entity& entity)
        {
            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen
                    ? rgbaToAbgr(110, 110, 110)
                    : rgbaToAbgr(120, 255, 150);

            case EntityType::Terminal:
                return entity.isPowered
                    ? rgbaToAbgr(90, 170, 255)
                    : rgbaToAbgr(70, 110, 150);

            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return rgbaToAbgr(220, 110, 110);
                }
                return entity.isOpen
                    ? rgbaToAbgr(160, 160, 210)
                    : rgbaToAbgr(190, 190, 240);

            default:
                return rgbaToAbgr(120, 255, 150);
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
        TileCoord hoveredTile)
    {
        BgfxWorldRenderData data{};

        data.tiles.quads.reserve(
            static_cast<size_t>(worldState.world().getWidth())
            * static_cast<size_t>(worldState.world().getHeight()));

        for (int y = 0; y < worldState.world().getHeight(); ++y)
        {
            for (int x = 0; x < worldState.world().getWidth(); ++x)
            {
                const TileCoord tile{ x, y };
                data.tiles.quads.push_back(BgfxQuad{
                    tileToScreenRect(worldState, camera, tile),
                    tileColor(worldState.world().isBlocked(tile))
                });
            }
        }

        if (pathIndex < currentPath.size())
        {
            data.path.quads.reserve(currentPath.size() - pathIndex);
        }

        for (size_t i = pathIndex; i < currentPath.size(); ++i)
        {
            data.path.quads.push_back(BgfxQuad{
                centeredScreenRect(
                    camera,
                    worldState.world().tileToWorldCenter(currentPath[i]),
                    5.0f * camera.getZoom()),
                pathColor()
            });
        }

        if (hasHoveredTile && worldState.world().isInBounds(hoveredTile))
        {
            data.hoveredTile.quads.push_back(BgfxQuad{
                tileToScreenRect(worldState, camera, hoveredTile),
                hoveredColor(worldState.world().isBlocked(hoveredTile))
            });
        }

        data.entities.quads.reserve(worldState.entities().all().size());

        for (const Entity& entity : worldState.entities().all())
        {
            data.entities.quads.push_back(BgfxQuad{
                centeredScreenRect(
                    camera,
                    worldState.world().tileToWorldCenter(entity.tile),
                    8.0f * camera.getZoom()),
                entityColor(entity)
            });
        }

        data.player.quads.push_back(BgfxQuad{
            centeredScreenRect(camera, playerPosition, 12.0f * camera.getZoom()),
            playerColor()
        });

        return data;
    }
}
