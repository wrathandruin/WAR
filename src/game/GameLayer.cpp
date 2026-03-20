#include "game/GameLayer.h"

#include <memory>
#include <windows.h>

#include "engine/render/BgfxRenderDevice.h"
#include "engine/render/GdiRenderDevice.h"
#include "engine/simulation/SimulationIntent.h"

namespace war
{
    void GameLayer::initialize(IWindow& window)
    {
        m_window = &window;
        m_camera.setViewportSize(window.getWidth(), window.getHeight());
        m_camera.setPosition({ 0.0f, 0.0f });

        m_simulationRuntime.initializeForLocalAuthority();

        const TileCoord spawnTile{ 2, 2 };
        m_selectedTile = spawnTile;
        m_hasSelectedTile = true;

        m_runtimeBoundaryReport = RuntimePaths::buildReport();
        RuntimePaths::ensureRuntimeDirectories(m_runtimeBoundaryReport);
        m_localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(m_runtimeBoundaryReport);
        LocalDemoDiagnostics::writeStartupReport(m_runtimeBoundaryReport, m_localDemoDiagnosticsReport);

        auto pushM33StartupEvents = [this]()
        {
            pushEvent("Milestone 33 initialized");
            pushEvent("shared simulation contract / fixed-step ownership active");
            pushEvent("Simulation owner: SharedSimulationRuntime");
            pushEvent("Client role: input, camera, render, diagnostics");
            pushEvent("Fixed-step cadence: 50 ms authoritative local tick");
            pushEvent(std::string("Build: ")
                + m_localDemoDiagnosticsReport.buildConfiguration
                + " | "
                + m_localDemoDiagnosticsReport.buildTimestamp);
            pushEvent(m_runtimeBoundaryReport.runningFromSourceTree
                ? "Runtime mode: source-tree layout"
                : "Runtime mode: packaged layout");
            pushEvent(std::string("Startup report: ")
                + RuntimePaths::displayPath(m_localDemoDiagnosticsReport.startupReportPath));
            pushEvent(m_localDemoDiagnosticsReport.packagedLaneReady
                ? "Packaged demo lane staged next to executable"
                : "Packaged demo lane not yet staged next to executable");
            pushEvent("Press O / H / 7 / 8 / 9 for review overlays and palette modes");
        };

        auto preferred = std::make_unique<BgfxRenderDevice>();
        if (preferred->initialize(m_window->getHandle()))
        {
            m_renderDevice = std::move(preferred);
            pushM33StartupEvents();
            pushEvent(std::string("Active backend: ") + m_renderDevice->name());
            return;
        }

        auto fallback = std::make_unique<GdiRenderDevice>();
        const bool fallbackReady = fallback->initialize(m_window->getHandle());
        m_renderDevice = std::move(fallback);

        pushM33StartupEvents();
        pushEvent("bgfx unavailable, falling back to GDI");
        pushEvent(std::string("Active backend: ") + m_renderDevice->name());
        if (!fallbackReady)
        {
            pushEvent("Warning: fallback backend failed to initialize");
        }
    }

    void GameLayer::update(float dt)
    {
        m_lastDeltaTime = dt;
        m_camera.setViewportSize(m_window->getWidth(), m_window->getHeight());

        updateInput();
        m_simulationRuntime.advanceFrame(dt);

        m_hasActionTargetTile = m_simulationRuntime.hasMovementTarget();
        if (m_hasActionTargetTile)
        {
            m_actionTargetTile = m_simulationRuntime.movementTargetTile();
        }
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

        const WorldState& worldState = m_simulationRuntime.worldState();
        const Vec2& playerPosition = m_simulationRuntime.presentedPlayerPosition();
        const std::vector<TileCoord>& currentPath = m_simulationRuntime.currentPath();
        const size_t pathIndex = m_simulationRuntime.pathIndex();
        const std::vector<std::string>& eventLog = m_simulationRuntime.eventLog();
        const SharedSimulationDiagnostics& simulationDiagnostics = m_simulationRuntime.diagnostics();

        HDC dc = m_renderDevice->getDrawContext();
        if (dc != nullptr)
        {
            m_worldRenderer.render(
                dc,
                clientRect,
                worldState,
                m_camera,
                playerPosition,
                currentPath,
                pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile);

            m_debugOverlayRenderer.render(
                dc,
                worldState,
                m_camera,
                playerPosition,
                currentPath,
                pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile,
                eventLog,
                m_lastDeltaTime,
                m_window->getMousePosition(),
                m_runtimeBoundaryReport,
                m_localDemoDiagnosticsReport,
                simulationDiagnostics);
        }
        else
        {
            m_bgfxWorldRenderer.render(
                worldState,
                m_camera,
                playerPosition,
                currentPath,
                pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile,
                m_runtimeBoundaryReport,
                m_localDemoDiagnosticsReport,
                simulationDiagnostics);

            m_bgfxDebugFrameRenderer.render(
                worldState,
                playerPosition,
                eventLog,
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

        const WorldState& worldState = m_simulationRuntime.worldState();

        const POINT mouse = m_window->getMousePosition();
        const Vec2 mouseWorld = m_camera.screenToWorld(mouse.x, mouse.y);
        const TileCoord hovered = worldState.world().worldToTile(mouseWorld);

        m_hasHoveredTile = worldState.world().isInBounds(hovered);
        m_hoveredTile = hovered;

        POINT click{};
        if (m_window->consumeLeftClick(click))
        {
            const Vec2 world = m_camera.screenToWorld(click.x, click.y);
            const TileCoord targetTile = worldState.world().worldToTile(world);

            m_selectedTile = targetTile;
            m_hasSelectedTile = worldState.world().isInBounds(targetTile);
            m_actionTargetTile = targetTile;
            m_hasActionTargetTile = m_hasSelectedTile;

            m_simulationRuntime.enqueueIntent(SimulationIntentType::MoveToTile, targetTile);
        }

        POINT rightClick{};
        if (m_window->consumeRightClick(rightClick))
        {
            const Vec2 world = m_camera.screenToWorld(rightClick.x, rightClick.y);
            const TileCoord targetTile = worldState.world().worldToTile(world);

            m_selectedTile = targetTile;
            m_hasSelectedTile = worldState.world().isInBounds(targetTile);

            const bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            if (shiftDown)
            {
                m_simulationRuntime.enqueueIntent(SimulationIntentType::InspectTile, targetTile);
            }
            else
            {
                m_simulationRuntime.enqueueIntent(SimulationIntentType::InteractTile, targetTile);
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

        WorldState& worldState = m_simulationRuntime.worldState();

        if (overlayDown && !m_overlayKeyWasDown)
        {
            const bool newState = !worldState.regionOverlayEnabled();
            worldState.setRegionOverlayEnabled(newState);
            pushEvent(newState ? "Region boundary overlay enabled" : "Region boundary overlay disabled");
        }

        if (hotspotDown && !m_hotspotKeyWasDown)
        {
            const bool newState = !worldState.authoringHotspotsVisible();
            worldState.setAuthoringHotspotsVisible(newState);
            pushEvent(newState ? "Authored hotspot overlay enabled" : "Authored hotspot overlay disabled");
        }

        if (palette7Down && !m_palette7WasDown)
        {
            worldState.setPaletteMode(BgfxThemePaletteMode::Default);
            pushEvent("Palette mode: Default");
        }

        if (palette8Down && !m_palette8WasDown)
        {
            worldState.setPaletteMode(BgfxThemePaletteMode::Muted);
            pushEvent("Palette mode: Muted");
        }

        if (palette9Down && !m_palette9WasDown)
        {
            worldState.setPaletteMode(BgfxThemePaletteMode::Vivid);
            pushEvent("Palette mode: Vivid");
        }

        m_overlayKeyWasDown = overlayDown;
        m_hotspotKeyWasDown = hotspotDown;
        m_palette7WasDown = palette7Down;
        m_palette8WasDown = palette8Down;
        m_palette9WasDown = palette9Down;
    }

    void GameLayer::pushEvent(const std::string& message)
    {
        m_simulationRuntime.appendEvent(message);
    }

    RECT GameLayer::getClientRect() const
    {
        RECT rect{};
        GetClientRect(m_window->getHandle(), &rect);
        return rect;
    }
}
