#pragma once

#include <string>
#include <vector>

#include <windows.h>

#include "engine/gameplay/ActionQueue.h"
#include "engine/gameplay/Entity.h"
#include "engine/math/Vec2.h"
#include "engine/render/Camera2D.h"
#include "engine/world/WorldGrid.h"

namespace war
{
    class Win32Window;

    class GameLayer
    {
    public:
        void initialize(Win32Window& window);
        void update(float dt);
        void render();
        void shutdown();

    private:
        void updateInput();
        void processActions();
        void updatePlayer(float dt);
        void pushEvent(const std::string& message);
        void rebuildPathTo(TileCoord targetTile);

        void drawWorld(HDC dc, const RECT& clientRect);
        void drawTiles(HDC dc);
        void drawHoveredTile(HDC dc);
        void drawPath(HDC dc);
        void drawPlayer(HDC dc);
        void drawEntities(HDC dc);
        void drawOverlay(HDC dc);

        [[nodiscard]] RECT getClientRect() const;
        [[nodiscard]] RECT tileToScreenRect(TileCoord tile) const;
        [[nodiscard]] Entity* getEntityAt(TileCoord tile);
        [[nodiscard]] const Entity* getEntityAt(TileCoord tile) const;

        Win32Window* m_window = nullptr;
        Camera2D m_camera{};
        WorldGrid m_world{ 48, 36, 48 };
        ActionQueue m_actions{};

        std::vector<Entity> m_entities;

        Vec2 m_playerPosition{ 0.0f, 0.0f };
        float m_playerSpeed = 210.0f;

        TileCoord m_hoveredTile{};
        bool m_hasHoveredTile = false;

        std::vector<TileCoord> m_currentPath;
        size_t m_pathIndex = 0;

        float m_lastDeltaTime = 0.016f;
        std::vector<std::string> m_eventLog;
    };
}
