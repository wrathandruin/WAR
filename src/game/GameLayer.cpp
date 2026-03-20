#include "game/GameLayer.h"

#include <cstdio>
#include <cstring>

#include <windows.h>

#include "engine/gameplay/Action.h"
#include "engine/gameplay/ActionSystem.h"
#include "platform/win32/Win32Window.h"

namespace war
{
    namespace
    {
        const char* entityTypeToText(EntityType type)
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
    }

    void GameLayer::initialize(Win32Window& window)
    {
        m_window = &window;
        m_camera.setViewportSize(window.getWidth(), window.getHeight());
        m_camera.setPosition({ 0.0f, 0.0f });

        m_worldState.initializeTestWorld();

        const TileCoord spawnTile{ 2, 2 };
        m_playerPosition = m_worldState.world().tileToWorldCenter(spawnTile);

        pushEvent("Milestone 6 initialized");
        pushEvent("WorldState ready");
        pushEvent("EntityManager ready");
        pushEvent("ActionSystem ready");
    }

    void GameLayer::update(float dt)
    {
        m_lastDeltaTime = dt;
        m_camera.setViewportSize(m_window->getWidth(), m_window->getHeight());

        updateInput();
        ActionSystem::processPending(
            m_worldState,
            m_actions,
            m_playerPosition,
            m_currentPath,
            m_pathIndex,
            m_eventLog);
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

        if (width <= 0 || height <= 0)
        {
            ReleaseDC(hwnd, windowDc);
            return;
        }

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

    void GameLayer::updateInput()
    {
        const POINT mouse = m_window->getMousePosition();
        const Vec2 mouseWorld = m_camera.screenToWorld(mouse.x, mouse.y);
        const TileCoord hovered = m_worldState.world().worldToTile(mouseWorld);

        m_hasHoveredTile = m_worldState.world().isInBounds(hovered);
        m_hoveredTile = hovered;

        POINT click{};
        if (m_window->consumeLeftClick(click))
        {
            const Vec2 world = m_camera.screenToWorld(click.x, click.y);
            const TileCoord targetTile = m_worldState.world().worldToTile(world);
            m_actions.push({ ActionType::Move, targetTile });
        }

        POINT rightClick{};
        if (m_window->consumeRightClick(rightClick))
        {
            const Vec2 world = m_camera.screenToWorld(rightClick.x, rightClick.y);
            const TileCoord targetTile = m_worldState.world().worldToTile(world);

            const bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            if (shiftDown)
            {
                m_actions.push({ ActionType::Inspect, targetTile });
            }
            else
            {
                m_actions.push({ ActionType::Interact, targetTile });
            }
        }

        const int wheel = m_window->consumeMouseWheelDelta();
        if (wheel != 0)
        {
            m_camera.zoomBy(wheel > 0 ? 0.10f : -0.10f);
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
        if (m_currentPath.empty() || m_pathIndex >= m_currentPath.size())
        {
            return;
        }

        const Vec2 waypoint = m_worldState.world().tileToWorldCenter(m_currentPath[m_pathIndex]);
        const Vec2 toTarget = waypoint - m_playerPosition;
        const float distance = length(toTarget);

        if (distance < 1.0f)
        {
            m_playerPosition = waypoint;
            ++m_pathIndex;

            if (m_pathIndex >= m_currentPath.size())
            {
                pushEvent("Path complete");
            }
            return;
        }

        const Vec2 direction = normalize(toTarget);
        const float step = m_playerSpeed * dt;

        if (step >= distance)
        {
            m_playerPosition = waypoint;
            ++m_pathIndex;

            if (m_pathIndex >= m_currentPath.size())
            {
                pushEvent("Path complete");
            }
            return;
        }

        m_playerPosition += direction * step;
    }

    void GameLayer::pushEvent(const std::string& message)
    {
        m_eventLog.push_back(message);
        constexpr size_t kMaxEvents = 10;
        if (m_eventLog.size() > kMaxEvents)
        {
            m_eventLog.erase(
                m_eventLog.begin(),
                m_eventLog.begin() + static_cast<std::ptrdiff_t>(m_eventLog.size() - kMaxEvents));
        }
    }

    void GameLayer::drawWorld(HDC dc, const RECT& clientRect)
    {
        HBRUSH background = CreateSolidBrush(RGB(16, 18, 24));
        FillRect(dc, &clientRect, background);
        DeleteObject(background);

        drawTiles(dc);
        drawPath(dc);
        drawHoveredTile(dc);
        drawEntities(dc);
        drawPlayer(dc);
        drawOverlay(dc);
    }

    void GameLayer::drawTiles(HDC dc)
    {
        for (int y = 0; y < m_worldState.world().getHeight(); ++y)
        {
            for (int x = 0; x < m_worldState.world().getWidth(); ++x)
            {
                const TileCoord tile{ x, y };
                const RECT rect = tileToScreenRect(tile);
                const bool blocked = m_worldState.world().isBlocked(tile);

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

    void GameLayer::drawHoveredTile(HDC dc)
    {
        if (!m_hasHoveredTile || !m_worldState.world().isInBounds(m_hoveredTile))
        {
            return;
        }

        const RECT rect = tileToScreenRect(m_hoveredTile);
        HPEN pen = CreatePen(
            PS_SOLID,
            2,
            m_worldState.world().isBlocked(m_hoveredTile) ? RGB(220, 100, 100) : RGB(230, 220, 110));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(HOLLOW_BRUSH)));

        Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }

    void GameLayer::drawPath(HDC dc)
    {
        if (m_currentPath.empty())
        {
            return;
        }

        HBRUSH brush = CreateSolidBrush(RGB(255, 180, 90));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 210, 130));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));

        for (size_t i = m_pathIndex; i < m_currentPath.size(); ++i)
        {
            const Vec2 screen = m_camera.worldToScreen(m_worldState.world().tileToWorldCenter(m_currentPath[i]));
            const int radius = static_cast<int>(5.0f * m_camera.getZoom());
            Ellipse(dc,
                static_cast<int>(screen.x) - radius,
                static_cast<int>(screen.y) - radius,
                static_cast<int>(screen.x) + radius,
                static_cast<int>(screen.y) + radius);

            if (i > m_pathIndex)
            {
                const Vec2 prev = m_camera.worldToScreen(m_worldState.world().tileToWorldCenter(m_currentPath[i - 1]));
                MoveToEx(dc, static_cast<int>(prev.x), static_cast<int>(prev.y), nullptr);
                LineTo(dc, static_cast<int>(screen.x), static_cast<int>(screen.y));
            }
        }

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(brush);
        DeleteObject(pen);
    }

    void GameLayer::drawPlayer(HDC dc)
    {
        const Vec2 screen = m_camera.worldToScreen(m_playerPosition);
        const int radius = static_cast<int>(12.0f * m_camera.getZoom());

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

    void GameLayer::drawEntities(HDC dc)
    {
        HBRUSH brush = CreateSolidBrush(RGB(120, 255, 150));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(200, 255, 210));
        HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));

        for (const Entity& entity : m_worldState.entities().all())
        {
            const Vec2 screen = m_camera.worldToScreen(m_worldState.world().tileToWorldCenter(entity.tile));
            const int halfSize = static_cast<int>(8.0f * m_camera.getZoom());

            Rectangle(dc,
                static_cast<int>(screen.x) - halfSize,
                static_cast<int>(screen.y) - halfSize,
                static_cast<int>(screen.x) + halfSize,
                static_cast<int>(screen.y) + halfSize);
        }

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(brush);
        DeleteObject(pen);
    }

    void GameLayer::drawOverlay(HDC dc)
    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(225, 225, 225));

        const POINT mouse = m_window->getMousePosition();
        const Vec2 mouseWorld = m_camera.screenToWorld(mouse.x, mouse.y);
        const TileCoord mouseTile = m_worldState.world().worldToTile(mouseWorld);
        const TileCoord playerTile = m_worldState.world().worldToTile(m_playerPosition);
        const bool hoveredBlocked =
            m_worldState.world().isInBounds(mouseTile) && m_worldState.world().isBlocked(mouseTile);
        const Entity* hoveredEntity =
            m_worldState.world().isInBounds(mouseTile) ? m_worldState.entities().getAt(mouseTile) : nullptr;

        char buffer[896]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "WAR Milestone 6\n"
            "LMB: move    RMB: interact    Shift+RMB: inspect    MMB drag: pan    Wheel: zoom\n"
            "Player world: (%.1f, %.1f)\n"
            "Player tile: (%d, %d)\n"
            "Mouse tile: (%d, %d)\n"
            "Hovered blocked: %s\n"
            "Hovered entity: %s\n"
            "Hovered entity type: %s\n"
            "Camera: (%.1f, %.1f)  Zoom: %.2f\n"
            "Path nodes remaining: %zu\n"
            "Queued actions: %s\n"
            "Entities: %zu\n"
            "Frame dt: %.4f",
            m_playerPosition.x,
            m_playerPosition.y,
            playerTile.x,
            playerTile.y,
            mouseTile.x,
            mouseTile.y,
            hoveredBlocked ? "yes" : "no",
            hoveredEntity ? hoveredEntity->name.c_str() : "none",
            hoveredEntity ? entityTypeToText(hoveredEntity->type) : "none",
            m_camera.getPosition().x,
            m_camera.getPosition().y,
            m_camera.getZoom(),
            m_pathIndex < m_currentPath.size() ? m_currentPath.size() - m_pathIndex : 0,
            m_actions.hasActions() ? "yes" : "no",
            m_worldState.entities().count(),
            m_lastDeltaTime);

        TextOutA(dc, 16, 16, buffer, static_cast<int>(std::strlen(buffer)));

        int y = 224;
        TextOutA(dc, 16, y, "Event Log:", 10);
        y += 22;

        for (const std::string& entry : m_eventLog)
        {
            TextOutA(dc, 16, y, entry.c_str(), static_cast<int>(entry.size()));
            y += 18;
        }
    }

    RECT GameLayer::getClientRect() const
    {
        RECT rect{};
        GetClientRect(m_window->getHandle(), &rect);
        return rect;
    }

    RECT GameLayer::tileToScreenRect(TileCoord tile) const
    {
        const int tileSize = m_worldState.world().getTileSize();

        const Vec2 center = m_worldState.world().tileToWorldCenter(tile);
        const Vec2 topLeftWorld{
            center.x - static_cast<float>(tileSize) * 0.5f,
            center.y - static_cast<float>(tileSize) * 0.5f
        };
        const Vec2 bottomRightWorld{
            center.x + static_cast<float>(tileSize) * 0.5f,
            center.y + static_cast<float>(tileSize) * 0.5f
        };

        const Vec2 topLeft = m_camera.worldToScreen(topLeftWorld);
        const Vec2 bottomRight = m_camera.worldToScreen(bottomRightWorld);

        return RECT{
            static_cast<LONG>(topLeft.x),
            static_cast<LONG>(topLeft.y),
            static_cast<LONG>(bottomRight.x),
            static_cast<LONG>(bottomRight.y)
        };
    }
}
