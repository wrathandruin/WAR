#include "game/GameLayer.h"

#include <memory>
#include <windows.h>

#include "engine/gameplay/Action.h"
#include "engine/gameplay/ActionSystem.h"
#include "engine/render/BgfxRenderDevice.h"
#include "engine/render/BgfxWorldTheme.h"
#include "engine/render/GdiRenderDevice.h"

namespace war
{
    void GameLayer::initialize(IWindow& window)
    {
        m_window = &window;
        m_camera.setViewportSize(window.getWidth(), window.getHeight());
        m_camera.setPosition({ 0.0f, 0.0f });

        m_worldState.initializeTestWorld();

        const TileCoord spawnTile{ 2, 2 };
        m_playerPosition = m_worldState.world().tileToWorldCenter(spawnTile);

        {
            auto preferred = std::make_unique<BgfxRenderDevice>();
            if (preferred->initialize(m_window->getHandle()))
            {
                m_renderDevice = std::move(preferred);
                pushEvent("Milestone 24 initialized");
                pushEvent("bgfx theme sets / authoring hooks active");
                pushEvent(std::string("Active backend: ") + m_renderDevice->name());
                pushEvent(std::string("Theme: ") + BgfxWorldTheme::debugName(m_worldState.visualTheme()));
            }
            else
            {
                auto fallback = std::make_unique<GdiRenderDevice>();
                const bool fallbackReady = fallback->initialize(m_window->getHandle());
                m_renderDevice = std::move(fallback);

                pushEvent("Milestone 24 initialized");
                pushEvent("bgfx unavailable, falling back to GDI");
                pushEvent(std::string("Active backend: ") + m_renderDevice->name());
                if (!fallbackReady)
                {
                    pushEvent("Warning: fallback backend failed to initialize");
                }
            }
        }
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
        if (!m_renderDevice)
        {
            return;
        }

        const RECT clientRect = getClientRect();
        if (!m_renderDevice->beginFrame(m_window->getHandle(), clientRect))
        {
            return;
        }

        HDC dc = m_renderDevice->getDrawContext();
        if (dc != nullptr)
        {
            m_worldRenderer.render(
                dc,
                clientRect,
                m_worldState,
                m_camera,
                m_playerPosition,
                m_currentPath,
                m_pathIndex,
                m_hasHoveredTile,
                m_hoveredTile);

            m_debugOverlayRenderer.render(
                dc,
                m_worldState,
                m_camera,
                m_playerPosition,
                m_currentPath,
                m_pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_eventLog,
                m_lastDeltaTime,
                m_window->getMousePosition());
        }
        else
        {
            m_bgfxWorldRenderer.render(
                m_worldState,
                m_camera,
                m_playerPosition,
                m_currentPath,
                m_pathIndex,
                m_hasHoveredTile,
                m_hoveredTile);

            m_bgfxDebugFrameRenderer.render(
                m_worldState,
                m_playerPosition,
                m_eventLog,
                m_lastDeltaTime,
                m_bgfxWorldRenderer.statusMessage());
        }

        m_renderDevice->endFrame(m_window->getHandle());
    }

    void GameLayer::shutdown()
    {
        m_bgfxWorldRenderer.shutdown();

        if (m_renderDevice)
        {
            m_renderDevice->shutdown();
        }
    }

    void GameLayer::updateInput()
    {
        applyThemeHotkeys();

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

    void GameLayer::applyThemeHotkeys()
    {
        const bool key1Down = (GetAsyncKeyState('1') & 0x8000) != 0;
        const bool key2Down = (GetAsyncKeyState('2') & 0x8000) != 0;
        const bool key3Down = (GetAsyncKeyState('3') & 0x8000) != 0;

        if (key1Down && !m_themeKey1WasDown)
        {
            m_worldState.setVisualTheme(BgfxWorldThemeId::Industrial);
            pushEvent("Theme set: Industrial");
        }

        if (key2Down && !m_themeKey2WasDown)
        {
            m_worldState.setVisualTheme(BgfxWorldThemeId::Sterile);
            pushEvent("Theme set: Sterile");
        }

        if (key3Down && !m_themeKey3WasDown)
        {
            m_worldState.setVisualTheme(BgfxWorldThemeId::Emergency);
            pushEvent("Theme set: Emergency");
        }

        m_themeKey1WasDown = key1Down;
        m_themeKey2WasDown = key2Down;
        m_themeKey3WasDown = key3Down;
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

    RECT GameLayer::getClientRect() const
    {
        RECT rect{};
        GetClientRect(m_window->getHandle(), &rect);
        return rect;
    }
}
