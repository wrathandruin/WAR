#include "game/GameLayer.h"

#include <windows.h>

#include <cmath>
#include <cstdio>

#include "engine/math/Vec2.h"
#include "platform/win32/Win32Window.h"

namespace war
{
    namespace
    {
        constexpr int kGridSize = 64;
        constexpr int kGridExtent = 80;
    }

    void GameLayer::initialize(Win32Window& window)
    {
        m_window = &window;
        m_camera.setViewportSize(window.getWidth(), window.getHeight());
        m_camera.setPosition({ 0.0f, 0.0f });

        m_playerPosition = { 0.0f, 0.0f };
        m_moveTarget = m_playerPosition;
        m_lastClickWorld = m_playerPosition;
    }

    void GameLayer::update(float dt)
    {
        m_lastDeltaTime = dt;
        m_camera.setViewportSize(m_window->getWidth(), m_window->getHeight());

        updateInput(dt);
        updatePlayer(dt);
    }

    void GameLayer::render()
    {
        HWND hwnd = m_window->getHandle();
        HDC windowDc = GetDC(hwnd);
        if (windowDc == nullptr)
        {
            return;
        }

        RECT clientRect = getClientRect();
        const int width = clientRect.right - clientRect.left;
        const int height = clientRect.bottom - clientRect.top;

        HDC memoryDc = CreateCompatibleDC(windowDc);
        HBITMAP backBuffer = CreateCompatibleBitmap(windowDc, width, height);
        HGDIOBJ oldBitmap = SelectObject(memoryDc, backBuffer);

        drawWorld(memoryDc, clientRect);

        BitBlt(windowDc, 0, 0, width, height, memoryDc, 0, 0, SRCCOPY);

        SelectObject(memoryDc, oldBitmap);
        DeleteObject(backBuffer);
        DeleteDC(memoryDc);
        ReleaseDC(hwnd, windowDc);
    }

    void GameLayer::shutdown()
    {
    }

    void GameLayer::updateInput(float)
    {
        POINT click{};
        if (m_window->consumeLeftClick(click))
        {
            const Vec2 world = m_camera.screenToWorld(click.x, click.y);
            m_moveTarget = world;
            m_lastClickWorld = world;
            m_hasMoveTarget = true;
        }

        const int wheel = m_window->consumeMouseWheelDelta();
        if (wheel != 0)
        {
            const float zoomStep = wheel > 0 ? 0.10f : -0.10f;
            m_camera.zoomBy(zoomStep);
        }

        if (m_window->isMiddleMouseDown())
        {
            const POINT delta = m_window->consumeMouseDelta();
            const float zoom = m_camera.getZoom();
            m_camera.pan({
                -static_cast<float>(delta.x) / zoom,
                -static_cast<float>(delta.y) / zoom
            });
        }
        else
        {
            (void)m_window->consumeMouseDelta();
        }
    }

    void GameLayer::updatePlayer(float dt)
    {
        if (!m_hasMoveTarget)
        {
            return;
        }

        const Vec2 toTarget = m_moveTarget - m_playerPosition;
        const float distance = length(toTarget);

        if (distance < 2.0f)
        {
            m_playerPosition = m_moveTarget;
            m_hasMoveTarget = false;
            return;
        }

        const Vec2 direction = normalize(toTarget);
        const float step = m_playerSpeed * dt;

        if (step >= distance)
        {
            m_playerPosition = m_moveTarget;
            m_hasMoveTarget = false;
            return;
        }

        m_playerPosition += direction * step;
    }

    void GameLayer::drawWorld(HDC dc, const RECT& clientRect)
    {
        HBRUSH background = CreateSolidBrush(RGB(18, 18, 22));
        FillRect(dc, &clientRect, background);
        DeleteObject(background);

        drawGrid(dc, clientRect);
        drawDestination(dc);
        drawPlayer(dc);
        drawOverlay(dc);
    }

    void GameLayer::drawGrid(HDC dc, const RECT&)
    {
        HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(45, 45, 55));
        HPEN originPen = CreatePen(PS_SOLID, 1, RGB(90, 90, 120));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, gridPen));

        for (int i = -kGridExtent; i <= kGridExtent; ++i)
        {
            const float worldX = static_cast<float>(i * kGridSize);
            const Vec2 top = m_camera.worldToScreen({ worldX, static_cast<float>(-kGridExtent * kGridSize) });
            const Vec2 bottom = m_camera.worldToScreen({ worldX, static_cast<float>(kGridExtent * kGridSize) });

            MoveToEx(dc, static_cast<int>(top.x), static_cast<int>(top.y), nullptr);
            LineTo(dc, static_cast<int>(bottom.x), static_cast<int>(bottom.y));
        }

        for (int i = -kGridExtent; i <= kGridExtent; ++i)
        {
            const float worldY = static_cast<float>(i * kGridSize);
            const Vec2 left = m_camera.worldToScreen({ static_cast<float>(-kGridExtent * kGridSize), worldY });
            const Vec2 right = m_camera.worldToScreen({ static_cast<float>(kGridExtent * kGridSize), worldY });

            MoveToEx(dc, static_cast<int>(left.x), static_cast<int>(left.y), nullptr);
            LineTo(dc, static_cast<int>(right.x), static_cast<int>(right.y));
        }

        SelectObject(dc, originPen);
        const Vec2 h1 = m_camera.worldToScreen({ -2048.0f, 0.0f });
        const Vec2 h2 = m_camera.worldToScreen({ 2048.0f, 0.0f });
        MoveToEx(dc, static_cast<int>(h1.x), static_cast<int>(h1.y), nullptr);
        LineTo(dc, static_cast<int>(h2.x), static_cast<int>(h2.y));

        const Vec2 v1 = m_camera.worldToScreen({ 0.0f, -2048.0f });
        const Vec2 v2 = m_camera.worldToScreen({ 0.0f, 2048.0f });
        MoveToEx(dc, static_cast<int>(v1.x), static_cast<int>(v1.y), nullptr);
        LineTo(dc, static_cast<int>(v2.x), static_cast<int>(v2.y));

        SelectObject(dc, oldPen);
        DeleteObject(gridPen);
        DeleteObject(originPen);
    }

    void GameLayer::drawPlayer(HDC dc)
    {
        const Vec2 screen = m_camera.worldToScreen(m_playerPosition);
        const int radius = static_cast<int>(12.0f * m_camera.getZoom());

        HBRUSH brush = CreateSolidBrush(RGB(170, 210, 255));
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

    void GameLayer::drawDestination(HDC dc)
    {
        const Vec2 screen = m_camera.worldToScreen(m_lastClickWorld);
        const int size = static_cast<int>(8.0f * m_camera.getZoom());

        HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 180, 110));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));

        MoveToEx(dc, static_cast<int>(screen.x) - size, static_cast<int>(screen.y), nullptr);
        LineTo(dc, static_cast<int>(screen.x) + size, static_cast<int>(screen.y));
        MoveToEx(dc, static_cast<int>(screen.x), static_cast<int>(screen.y) - size, nullptr);
        LineTo(dc, static_cast<int>(screen.x), static_cast<int>(screen.y) + size);

        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }

    void GameLayer::drawOverlay(HDC dc)
    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(220, 220, 220));

        const POINT mouse = m_window->getMousePosition();
        const Vec2 mouseWorld = m_camera.screenToWorld(mouse.x, mouse.y);

        char buffer[512]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "WAR Milestone 1\n"
            "LMB: move    MMB drag: pan    Wheel: zoom\n"
            "Player: (%.1f, %.1f)\n"
            "Target: (%.1f, %.1f)\n"
            "Mouse world: (%.1f, %.1f)\n"
            "Camera: (%.1f, %.1f)  Zoom: %.2f\n"
            "Frame dt: %.4f",
            m_playerPosition.x,
            m_playerPosition.y,
            m_moveTarget.x,
            m_moveTarget.y,
            mouseWorld.x,
            mouseWorld.y,
            m_camera.getPosition().x,
            m_camera.getPosition().y,
            m_camera.getZoom(),
            m_lastDeltaTime);

        TextOutA(dc, 16, 16, buffer, static_cast<int>(std::strlen(buffer)));
    }

    RECT GameLayer::getClientRect() const
    {
        RECT rect{};
        GetClientRect(m_window->getHandle(), &rect);
        return rect;
    }
}
