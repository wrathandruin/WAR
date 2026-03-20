#pragma once

#include <windows.h>

#include "engine/math/Vec2.h"
#include "engine/render/Camera2D.h"

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
        void updateInput(float dt);
        void updatePlayer(float dt);
        void drawWorld(HDC dc, const RECT& clientRect);
        void drawGrid(HDC dc, const RECT& clientRect);
        void drawPlayer(HDC dc);
        void drawDestination(HDC dc);
        void drawOverlay(HDC dc);
        [[nodiscard]] RECT getClientRect() const;

        Win32Window* m_window = nullptr;
        Camera2D m_camera{};

        Vec2 m_playerPosition{ 0.0f, 0.0f };
        Vec2 m_moveTarget{ 0.0f, 0.0f };
        bool m_hasMoveTarget = false;
        float m_playerSpeed = 240.0f;

        Vec2 m_lastClickWorld{};
        float m_lastDeltaTime = 0.016f;
    };
}
