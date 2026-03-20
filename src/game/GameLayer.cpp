#include "game/GameLayer.h"

#include <memory>
#include <windows.h>

#include "engine/gameplay/Action.h"
#include "engine/gameplay/ActionSystem.h"
#include "engine/render/BgfxRenderDevice.h"
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
        m_selectedTile = spawnTile;
        m_hasSelectedTile = true;

        {
            auto preferred = std::make_unique<BgfxRenderDevice>();
            if (preferred->initialize(m_window->getHandle()))
            {
                m_renderDevice = std::move(preferred);
                pushEvent("Milestone 30 initialized");
                pushEvent("playable slice readability / interaction affordances active");
                pushEvent("Hover, selection, and move-target readability now support short demos more cleanly");
                pushEvent("Inspect and interact messaging now name concrete entities and authored anchors");
                pushEvent("Region boundary overlay is enabled by default");
                pushEvent("Authored hotspot overlay is enabled by default");
                pushEvent("Press O to toggle region boundary overlay");
                pushEvent("Press H to toggle authored hotspot overlay");
                pushEvent("Press 7 / 8 / 9 for Default / Muted / Vivid palette");
                pushEvent(std::string("Active backend: ") + m_renderDevice->name());
            }
            else
            {
                auto fallback = std::make_unique<GdiRenderDevice>();
                const bool fallbackReady = fallback->initialize(m_window->getHandle());
                m_renderDevice = std::move(fallback);

                pushEvent("Milestone 30 initialized");
                pushEvent("bgfx unavailable, falling back to GDI");
                pushEvent("playable slice readability / interaction affordances active");
                pushEvent("Hover, selection, and move-target readability now support short demos more cleanly");
                pushEvent("Inspect and interact messaging now name concrete entities and authored anchors");
                pushEvent("Region boundary overlay is enabled by default");
                pushEvent("Authored hotspot overlay is enabled by default");
                pushEvent("Press O to toggle region boundary overlay");
                pushEvent("Press H to toggle authored hotspot overlay");
                pushEvent("Press 7 / 8 / 9 for Default / Muted / Vivid palette");
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
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile);

            m_debugOverlayRenderer.render(
                dc,
                m_worldState,
                m_camera,
                m_playerPosition,
                m_currentPath,
                m_pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile,
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
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile);

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
        applyAuthoringHotkeys();

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

            m_selectedTile = targetTile;
            m_hasSelectedTile = m_worldState.world().isInBounds(targetTile);
            m_actionTargetTile = targetTile;
            m_hasActionTargetTile = m_hasSelectedTile;

            m_actions.push({ ActionType::Move, targetTile });
        }

        POINT rightClick{};
        if (m_window->consumeRightClick(rightClick))
        {
            const Vec2 world = m_camera.screenToWorld(rightClick.x, rightClick.y);
            const TileCoord targetTile = m_worldState.world().worldToTile(world);

            m_selectedTile = targetTile;
            m_hasSelectedTile = m_worldState.world().isInBounds(targetTile);

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

    void GameLayer::applyAuthoringHotkeys()
    {
        const bool overlayDown = (GetAsyncKeyState('O') & 0x8000) != 0;
        const bool hotspotDown = (GetAsyncKeyState('H') & 0x8000) != 0;
        const bool palette7Down = (GetAsyncKeyState('7') & 0x8000) != 0;
        const bool palette8Down = (GetAsyncKeyState('8') & 0x8000) != 0;
        const bool palette9Down = (GetAsyncKeyState('9') & 0x8000) != 0;

        if (overlayDown && !m_overlayKeyWasDown)
        {
            const bool newState = !m_worldState.regionOverlayEnabled();
            m_worldState.setRegionOverlayEnabled(newState);
            pushEvent(newState ? "Region boundary overlay enabled" : "Region boundary overlay disabled");
        }

        if (hotspotDown && !m_hotspotKeyWasDown)
        {
            const bool newState = !m_worldState.authoringHotspotsVisible();
            m_worldState.setAuthoringHotspotsVisible(newState);
            pushEvent(newState ? "Authored hotspot overlay enabled" : "Authored hotspot overlay disabled");
        }

        if (palette7Down && !m_palette7WasDown)
        {
            m_worldState.setPaletteMode(BgfxThemePaletteMode::Default);
            pushEvent("Palette mode: Default");
        }

        if (palette8Down && !m_palette8WasDown)
        {
            m_worldState.setPaletteMode(BgfxThemePaletteMode::Muted);
            pushEvent("Palette mode: Muted");
        }

        if (palette9Down && !m_palette9WasDown)
        {
            m_worldState.setPaletteMode(BgfxThemePaletteMode::Vivid);
            pushEvent("Palette mode: Vivid");
        }

        m_overlayKeyWasDown = overlayDown;
        m_hotspotKeyWasDown = hotspotDown;
        m_palette7WasDown = palette7Down;
        m_palette8WasDown = palette8Down;
        m_palette9WasDown = palette9Down;
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
                m_hasActionTargetTile = false;
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
                m_hasActionTargetTile = false;
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
