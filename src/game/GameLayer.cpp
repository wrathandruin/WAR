#include "game/GameLayer.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>

#include <windows.h>

#include "engine/host/HeadlessHostPresence.h"
#include "engine/render/BgfxRenderDevice.h"
#include "engine/render/GdiRenderDevice.h"

namespace war
{
    namespace
    {
        bool configsEqual(const ReplicationHarnessConfig& lhs, const ReplicationHarnessConfig& rhs)
        {
            return lhs.enabled == rhs.enabled
                && lhs.intentLatencyMilliseconds == rhs.intentLatencyMilliseconds
                && lhs.acknowledgementLatencyMilliseconds == rhs.acknowledgementLatencyMilliseconds
                && lhs.snapshotLatencyMilliseconds == rhs.snapshotLatencyMilliseconds
                && lhs.jitterMilliseconds == rhs.jitterMilliseconds;
        }

        std::string sanitizeSingleLine(const std::string& value)
        {
            std::string sanitized = value;
            for (char& ch : sanitized)
            {
                if (ch == '\n' || ch == '\r')
                {
                    ch = ' ';
                }
            }

            return sanitized;
        }

        std::string harnessSummary(const ReplicationHarnessConfig& config)
        {
            std::ostringstream summary;
            summary
                << (config.enabled ? "enabled" : "disabled")
                << " | latency ms: "
                << config.intentLatencyMilliseconds << "/"
                << config.acknowledgementLatencyMilliseconds << "/"
                << config.snapshotLatencyMilliseconds
                << " | jitter ms: " << config.jitterMilliseconds;
            return summary.str();
        }

        bool writeTextFileAtomically(const std::filesystem::path& path, const std::string& contents)
        {
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error)
            {
                return false;
            }

            const std::filesystem::path tempPath = path.parent_path() / (path.filename().string() + ".tmp");
            std::ofstream output(tempPath, std::ios::out | std::ios::trunc);
            if (!output.is_open())
            {
                return false;
            }

            output << contents;
            output.close();
            if (!output)
            {
                std::filesystem::remove(tempPath, error);
                return false;
            }

#if defined(_WIN32)
            if (!MoveFileExW(tempPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
                std::filesystem::remove(tempPath, error);
                return false;
            }
#else
            std::filesystem::rename(tempPath, path, error);
            if (error)
            {
                std::filesystem::remove(tempPath, error);
                return false;
            }
#endif

            return true;
        }

        bool sameSnapshotIdentity(
            const AuthoritativeWorldSnapshot& snapshot,
            uint64_t publishedEpochMilliseconds,
            uint64_t simulationTicks,
            uint64_t lastProcessedIntentSequence)
        {
            return snapshot.publishedEpochMilliseconds == publishedEpochMilliseconds
                && snapshot.simulationTicks == simulationTicks
                && snapshot.lastProcessedIntentSequence == lastProcessedIntentSequence;
        }

        void appendClientTrace(const RuntimeBoundaryReport& runtimeBoundaryReport, std::string_view line)
        {
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", line);
        }
    }

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
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize entered");
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize simulation initialized");
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize runtime paths ready");
        m_localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(m_runtimeBoundaryReport);
        LocalDemoDiagnostics::writeStartupReport(m_runtimeBoundaryReport, m_localDemoDiagnosticsReport);
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize startup report written");
        m_headlessHostPresenceReport = HeadlessHostPresence::buildReport(m_runtimeBoundaryReport);
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize headless host presence read");
        m_authoritativeHostProtocolReport = AuthoritativeHostProtocol::buildReport(m_runtimeBoundaryReport);
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize authoritative protocol report built");
        m_replicationHarnessConfig = ReplicationHarness::loadConfig(m_runtimeBoundaryReport);
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize harness config loaded");
        refreshAuthorityMode();
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize authority mode refreshed");
        updateReplicationDiagnostics();
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize replication diagnostics updated");
        writeClientReplicationStatus();
        appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize client replication status write attempted");

        auto pushM40StartupEvents = [this]()
        {
            pushEvent("Milestone 40 initialized");
            pushEvent("six-second combat / encounter resolution active");
            pushEvent("Simulation owner: SharedSimulationRuntime with authoritative localhost host lane");
            pushEvent("Headless host launch: WARServer.exe");
            pushEvent(std::string("Build: ")
                + m_localDemoDiagnosticsReport.buildConfiguration
                + " | "
                + m_localDemoDiagnosticsReport.buildTimestamp);
            pushEvent(m_runtimeBoundaryReport.runningFromSourceTree
                ? "Runtime mode: source-tree layout"
                : "Runtime mode: packaged layout");
            pushEvent(std::string("Startup report: ")
                + RuntimePaths::displayPath(m_localDemoDiagnosticsReport.startupReportPath));

            if (m_headlessHostPresenceReport.hostOnline)
            {
                pushEvent("Headless host heartbeat detected");
            }
            else if (m_headlessHostPresenceReport.statusFilePresent)
            {
                pushEvent("Headless host status file found but no fresh running heartbeat");
            }
            else
            {
                pushEvent("No external headless host heartbeat detected yet");
            }

            pushEvent("Move into encounter hotspots to trigger the first authoritative combat lane.");
            pushEvent("Press J / K / L for harness toggle, latency preset, and jitter preset");
        };

        auto preferred = std::make_unique<BgfxRenderDevice>();
        if (preferred->initialize(m_window->getHandle()))
        {
            m_renderDevice = std::move(preferred);
            pushM40StartupEvents();
            pushEvent(std::string("Active backend: ") + m_renderDevice->name());
            appendClientTrace(m_runtimeBoundaryReport, "GameLayer::initialize bgfx backend initialized");
            return;
        }

        auto fallback = std::make_unique<GdiRenderDevice>();
        const bool fallbackReady = fallback->initialize(m_window->getHandle());
        m_renderDevice = std::move(fallback);

        pushM40StartupEvents();
        pushEvent("bgfx unavailable, falling back to GDI");
        pushEvent(std::string("Active backend: ") + m_renderDevice->name());
        if (!fallbackReady)
        {
            pushEvent("Warning: fallback backend failed to initialize");
        }
        appendClientTrace(m_runtimeBoundaryReport, fallbackReady
            ? "GameLayer::initialize GDI fallback initialized"
            : "GameLayer::initialize GDI fallback failed");
    }

    void GameLayer::update(float dt)
    {
        m_lastDeltaTime = dt;
        m_camera.setViewportSize(m_window->getWidth(), m_window->getHeight());

        m_headlessHostPresenceReport = HeadlessHostPresence::buildReport(m_runtimeBoundaryReport);
        m_authoritativeHostProtocolReport = AuthoritativeHostProtocol::buildReport(m_runtimeBoundaryReport);
        refreshAuthorityMode();
        pollAuthoritativeHostResponses();
        updateReplicationDiagnostics();
        updateInput();
        m_simulationRuntime.advanceFrame(dt);

        m_hasActionTargetTile = m_simulationRuntime.hasMovementTarget();
        if (m_hasActionTargetTile)
        {
            m_actionTargetTile = m_simulationRuntime.movementTargetTile();
        }

        writeClientReplicationStatus();
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
                simulationDiagnostics,
                m_headlessHostPresenceReport,
                m_authoritativeHostProtocolReport);
        }
        else
        {
            const bool bgfxRendered = m_bgfxWorldRenderer.render(
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
                simulationDiagnostics,
                m_headlessHostPresenceReport,
                m_authoritativeHostProtocolReport);

            if (!bgfxRendered)
            {
                pushEvent(std::string("bgfx render status: ") + m_bgfxWorldRenderer.statusMessage());
            }

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

        auto submitIntent = [this](SimulationIntentType type, TileCoord target, const std::string& queueFailureMessage)
        {
            const uint64_t queuedSequence = m_simulationRuntime.enqueueIntent(type, target);
            if (queuedSequence == 0)
            {
                pushEvent(queueFailureMessage);
                return;
            }

            if (!m_useHeadlessHostAuthority)
            {
                return;
            }

            SimulationIntent intent{};
            intent.sequence = queuedSequence;
            intent.type = type;
            intent.target = target;

            std::string protocolError;
            if (AuthoritativeHostProtocol::writeIntentRequest(m_runtimeBoundaryReport, intent, protocolError))
            {
                return;
            }

            SimulationIntentAck rejection{};
            rejection.sequence = queuedSequence;
            rejection.type = type;
            rejection.target = target;
            rejection.result = SimulationIntentAckResult::Rejected;
            rejection.reason = std::string("host queue write failed: ") + protocolError;
            m_simulationRuntime.applyAcknowledgement(rejection);

            pushEvent(
                std::string("Failed to submit intent #")
                + std::to_string(queuedSequence)
                + " to headless host: "
                + protocolError);
        };

        POINT click{};
        if (m_window->consumeLeftClick(click))
        {
            const Vec2 world = m_camera.screenToWorld(click.x, click.y);
            const TileCoord targetTile = worldState.world().worldToTile(world);

            m_selectedTile = targetTile;
            m_hasSelectedTile = worldState.world().isInBounds(targetTile);
            m_actionTargetTile = targetTile;
            m_hasActionTargetTile = m_hasSelectedTile;

            submitIntent(SimulationIntentType::MoveToTile, targetTile, "Failed to queue move intent.");
        }

        POINT rightClick{};
        if (m_window->consumeRightClick(rightClick))
        {
            const Vec2 world = m_camera.screenToWorld(rightClick.x, rightClick.y);
            const TileCoord targetTile = worldState.world().worldToTile(world);

            m_selectedTile = targetTile;
            m_hasSelectedTile = worldState.world().isInBounds(targetTile);

            const bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            submitIntent(
                shiftDown ? SimulationIntentType::InspectTile : SimulationIntentType::InteractTile,
                targetTile,
                shiftDown ? "Failed to queue inspect intent." : "Failed to queue interact intent.");
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
            const POINT ignoredDelta = m_window->consumeMouseDelta();
            (void)ignoredDelta;
        }
    }

    void GameLayer::pushEvent(const std::string& message)
    {
        m_simulationRuntime.appendEvent(message);
    }

    void GameLayer::applyAuthoringHotkeys()
    {
        const bool overlayDown = (GetAsyncKeyState('O') & 0x8000) != 0;
        const bool hotspotDown = (GetAsyncKeyState('H') & 0x8000) != 0;
        const bool palette7Down = (GetAsyncKeyState('7') & 0x8000) != 0;
        const bool palette8Down = (GetAsyncKeyState('8') & 0x8000) != 0;
        const bool palette9Down = (GetAsyncKeyState('9') & 0x8000) != 0;
        const bool latencyToggleDown = (GetAsyncKeyState('J') & 0x8000) != 0;
        const bool latencyPresetDown = (GetAsyncKeyState('K') & 0x8000) != 0;
        const bool jitterPresetDown = (GetAsyncKeyState('L') & 0x8000) != 0;

        WorldState& worldState = m_simulationRuntime.worldState();
        bool harnessConfigChanged = false;

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

        if (latencyToggleDown && !m_latencyToggleWasDown)
        {
            m_replicationHarnessConfig.enabled = !m_replicationHarnessConfig.enabled;
            harnessConfigChanged = true;
            pushEvent(
                std::string("Replication harness ")
                + (m_replicationHarnessConfig.enabled ? "enabled" : "disabled")
                + " | "
                + harnessSummary(m_replicationHarnessConfig));
        }

        if (latencyPresetDown && !m_latencyPresetWasDown)
        {
            const uint32_t currentLatency = (std::max)(
                (std::max)(
                    m_replicationHarnessConfig.intentLatencyMilliseconds,
                    m_replicationHarnessConfig.acknowledgementLatencyMilliseconds),
                m_replicationHarnessConfig.snapshotLatencyMilliseconds);

            const uint32_t nextLatency = currentLatency == 0u
                ? 100u
                : (currentLatency <= 100u ? 350u : 0u);

            m_replicationHarnessConfig.intentLatencyMilliseconds = nextLatency;
            m_replicationHarnessConfig.acknowledgementLatencyMilliseconds = nextLatency;
            m_replicationHarnessConfig.snapshotLatencyMilliseconds = nextLatency;
            harnessConfigChanged = true;
            pushEvent(std::string("Replication latency preset: ") + std::to_string(nextLatency) + " ms");
        }

        if (jitterPresetDown && !m_jitterPresetWasDown)
        {
            const uint32_t currentJitter = m_replicationHarnessConfig.jitterMilliseconds;
            const uint32_t nextJitter = currentJitter == 0u
                ? 25u
                : (currentJitter <= 25u ? 100u : 0u);

            m_replicationHarnessConfig.jitterMilliseconds = nextJitter;
            harnessConfigChanged = true;
            pushEvent(std::string("Replication jitter preset: ") + std::to_string(nextJitter) + " ms");
        }

        if (harnessConfigChanged)
        {
            persistReplicationHarnessConfig();
            m_simulationRuntime.setReplicationHarnessState(
                m_replicationHarnessConfig.enabled,
                m_replicationHarnessConfig.intentLatencyMilliseconds,
                m_replicationHarnessConfig.acknowledgementLatencyMilliseconds,
                m_replicationHarnessConfig.snapshotLatencyMilliseconds,
                m_replicationHarnessConfig.jitterMilliseconds,
                m_lastSnapshotAgeMilliseconds);
        }

        m_overlayKeyWasDown = overlayDown;
        m_hotspotKeyWasDown = hotspotDown;
        m_palette7WasDown = palette7Down;
        m_palette8WasDown = palette8Down;
        m_palette9WasDown = palette9Down;
        m_latencyToggleWasDown = latencyToggleDown;
        m_latencyPresetWasDown = latencyPresetDown;
        m_jitterPresetWasDown = jitterPresetDown;
    }

    void GameLayer::refreshAuthorityMode()
    {
        const bool previousMode = m_useHeadlessHostAuthority;
        m_useHeadlessHostAuthority = m_headlessHostPresenceReport.hostOnline;

        m_simulationRuntime.setAuthorityMode(
            !m_useHeadlessHostAuthority,
            m_useHeadlessHostAuthority,
            m_useHeadlessHostAuthority);

        if (previousMode == m_useHeadlessHostAuthority)
        {
            return;
        }

        if (m_useHeadlessHostAuthority)
        {
            m_lastAppliedSnapshotPublishedEpochMilliseconds = 0;
            m_lastAppliedSnapshotSimulationTicks = 0;
            m_lastAppliedSnapshotSequence = 0;
            pushEvent("Authority mode: headless host online, client prediction + reconciliation active");
            return;
        }

        pushEvent("Authority mode: local fallback active");
    }

    void GameLayer::pollAuthoritativeHostResponses()
    {
        const std::vector<SimulationIntentAck> acknowledgements =
            AuthoritativeHostProtocol::collectAcknowledgementsForClient(m_runtimeBoundaryReport);
        for (const SimulationIntentAck& ack : acknowledgements)
        {
            m_simulationRuntime.applyAcknowledgement(ack);
        }

        if (!m_useHeadlessHostAuthority && !m_authoritativeHostProtocolReport.snapshotPresent)
        {
            m_lastSnapshotAgeMilliseconds = 0;
            if (!m_lastSnapshotReadError.empty())
            {
                m_lastSnapshotReadError.clear();
                m_simulationRuntime.clearSnapshotReadFailure();
            }
            return;
        }

        std::string snapshotError;
        const AuthoritativeWorldSnapshot snapshot =
            AuthoritativeHostProtocol::readAuthoritativeSnapshot(m_runtimeBoundaryReport, snapshotError);

        if (!snapshotError.empty())
        {
            m_lastSnapshotAgeMilliseconds = 0;
            if (snapshotError != m_lastSnapshotReadError)
            {
                m_simulationRuntime.recordSnapshotReadFailure(snapshotError);
                pushEvent(std::string("Snapshot read failed: ") + snapshotError);
                m_lastSnapshotReadError = snapshotError;
            }
            return;
        }

        if (!m_lastSnapshotReadError.empty())
        {
            pushEvent("Snapshot read recovered");
            m_lastSnapshotReadError.clear();
        }

        const uint64_t nowEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        if (snapshot.publishedEpochMilliseconds > 0 && nowEpochMilliseconds >= snapshot.publishedEpochMilliseconds)
        {
            m_lastSnapshotAgeMilliseconds = nowEpochMilliseconds - snapshot.publishedEpochMilliseconds;
        }
        else
        {
            m_lastSnapshotAgeMilliseconds = 0;
        }

        m_simulationRuntime.clearSnapshotReadFailure();

        if (!m_useHeadlessHostAuthority)
        {
            return;
        }

        if (sameSnapshotIdentity(
                snapshot,
                m_lastAppliedSnapshotPublishedEpochMilliseconds,
                m_lastAppliedSnapshotSimulationTicks,
                m_lastAppliedSnapshotSequence))
        {
            return;
        }

        std::string correctionReason;
        const bool corrected =
            m_simulationRuntime.applyAuthoritativeSnapshot(snapshot, m_lastSnapshotAgeMilliseconds, correctionReason);
        m_lastAppliedSnapshotPublishedEpochMilliseconds = snapshot.publishedEpochMilliseconds;
        m_lastAppliedSnapshotSimulationTicks = snapshot.simulationTicks;
        m_lastAppliedSnapshotSequence = snapshot.lastProcessedIntentSequence;
        if (corrected)
        {
            pushEvent(correctionReason);
        }
    }

    void GameLayer::persistReplicationHarnessConfig()
    {
        std::string error;
        if (!ReplicationHarness::saveConfig(m_runtimeBoundaryReport, m_replicationHarnessConfig, error))
        {
            pushEvent(std::string("Failed to persist replication harness config: ") + error);
        }
    }

    void GameLayer::updateReplicationDiagnostics()
    {
        const ReplicationHarnessConfig loadedConfig = ReplicationHarness::loadConfig(m_runtimeBoundaryReport);
        if (!configsEqual(loadedConfig, m_replicationHarnessConfig))
        {
            m_replicationHarnessConfig = loadedConfig;
            pushEvent(std::string("Replication harness config updated externally: ") + harnessSummary(m_replicationHarnessConfig));
        }

        m_simulationRuntime.setReplicationHarnessState(
            m_replicationHarnessConfig.enabled,
            m_replicationHarnessConfig.intentLatencyMilliseconds,
            m_replicationHarnessConfig.acknowledgementLatencyMilliseconds,
            m_replicationHarnessConfig.snapshotLatencyMilliseconds,
            m_replicationHarnessConfig.jitterMilliseconds,
            m_lastSnapshotAgeMilliseconds);
    }

    void GameLayer::writeClientReplicationStatus() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        std::ostringstream output;
        output
            << "version=2\n"
            << "authority_mode=" << (m_useHeadlessHostAuthority ? "headless-host" : "local") << "\n"
            << "runtime_mode=" << (m_runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged") << "\n"
            << "host_online=" << (m_headlessHostPresenceReport.hostOnline ? "yes" : "no") << "\n"
            << "status_parse_valid=" << (m_headlessHostPresenceReport.statusParseValid ? "yes" : "no") << "\n"
            << "heartbeat_fresh=" << (m_headlessHostPresenceReport.heartbeatFresh ? "yes" : "no") << "\n"
            << "protocol_lane_ready=" << (m_authoritativeHostProtocolReport.authorityLaneReady ? "yes" : "no") << "\n"
            << "snapshot_present=" << (m_authoritativeHostProtocolReport.snapshotPresent ? "yes" : "no") << "\n"
            << "latency_harness_enabled=" << (m_replicationHarnessConfig.enabled ? "yes" : "no") << "\n"
            << "intent_latency_ms=" << m_replicationHarnessConfig.intentLatencyMilliseconds << "\n"
            << "ack_latency_ms=" << m_replicationHarnessConfig.acknowledgementLatencyMilliseconds << "\n"
            << "snapshot_latency_ms=" << m_replicationHarnessConfig.snapshotLatencyMilliseconds << "\n"
            << "jitter_ms=" << m_replicationHarnessConfig.jitterMilliseconds << "\n"
            << "client_prediction_enabled=" << (diagnostics.clientPredictionEnabled ? "yes" : "no") << "\n"
            << "last_snapshot_age_ms=" << diagnostics.lastSnapshotAgeMilliseconds << "\n"
            << "last_snapshot_sequence=" << diagnostics.lastSnapshotSequence << "\n"
            << "last_snapshot_simulation_ticks=" << diagnostics.lastSnapshotSimulationTicks << "\n"
            << "last_position_drift_units=" << diagnostics.lastPositionDivergenceDistance << "\n"
            << "last_path_divergence=" << (diagnostics.lastPathDivergence ? "yes" : "no") << "\n"
            << "last_entity_divergence=" << (diagnostics.lastEntityDivergence ? "yes" : "no") << "\n"
            << "corrections_applied=" << diagnostics.correctionsApplied << "\n"
            << "divergence_events=" << diagnostics.divergenceEvents << "\n"
            << "snapshot_read_failures=" << diagnostics.snapshotReadFailures << "\n"
            << "last_snapshot_read_failed=" << (diagnostics.lastSnapshotReadFailed ? "yes" : "no") << "\n"
            << "last_snapshot_read_error=" << sanitizeSingleLine(diagnostics.lastSnapshotReadError) << "\n"
            << "persistence_active=" << (diagnostics.persistenceActive ? "yes" : "no") << "\n"
            << "persistence_data_loaded=" << (diagnostics.persistenceDataLoaded ? "yes" : "no") << "\n"
            << "persistence_schema_version=" << diagnostics.persistenceSchemaVersion << "\n"
            << "persistence_loaded_schema_version=" << diagnostics.persistenceLoadedSchemaVersion << "\n"
            << "persistence_migrated_from_version=" << diagnostics.persistenceMigratedFromSchemaVersion << "\n"
            << "persistence_migration_applied=" << (diagnostics.persistenceMigrationApplied ? "yes" : "no") << "\n"
            << "persistence_slot=" << sanitizeSingleLine(diagnostics.persistenceSlotName) << "\n"
            << "persistence_save_count=" << diagnostics.persistenceSaveCount << "\n"
            << "persistence_load_count=" << diagnostics.persistenceLoadCount << "\n"
            << "persistence_last_save_succeeded=" << (diagnostics.lastPersistenceSaveSucceeded ? "yes" : "no") << "\n"
            << "persistence_last_load_succeeded=" << (diagnostics.lastPersistenceLoadSucceeded ? "yes" : "no") << "\n"
            << "persistence_last_save_epoch_ms=" << diagnostics.lastPersistenceSaveEpochMilliseconds << "\n"
            << "persistence_last_load_epoch_ms=" << diagnostics.lastPersistenceLoadEpochMilliseconds << "\n"
            << "persistence_last_error=" << sanitizeSingleLine(diagnostics.lastPersistenceError) << "\n"
            << "host_pending_inbound_intents=" << m_headlessHostPresenceReport.pendingInboundIntentCount << "\n"
            << "host_pending_outbound_acks=" << m_headlessHostPresenceReport.pendingOutboundAcknowledgementCount << "\n"
            << "host_pending_snapshots=" << m_headlessHostPresenceReport.pendingSnapshotCount << "\n"
            << "host_persistence_save_present=" << (m_headlessHostPresenceReport.persistenceSavePresent ? "yes" : "no") << "\n"
            << "host_persistence_schema_version=" << m_headlessHostPresenceReport.persistenceSchemaVersion << "\n"
            << "host_persistence_loaded_schema_version=" << m_headlessHostPresenceReport.persistenceLoadedSchemaVersion << "\n"
            << "host_persistence_migrated_from_version=" << m_headlessHostPresenceReport.persistenceMigratedFromSchemaVersion << "\n"
            << "host_persistence_save_count=" << m_headlessHostPresenceReport.persistenceSaveCount << "\n"
            << "host_persistence_load_count=" << m_headlessHostPresenceReport.persistenceLoadCount << "\n"
            << "host_persistence_last_save_succeeded=" << (m_headlessHostPresenceReport.persistenceLastSaveSucceeded ? "yes" : "no") << "\n"
            << "host_persistence_last_load_succeeded=" << (m_headlessHostPresenceReport.persistenceLastLoadSucceeded ? "yes" : "no") << "\n"
            << "host_persistence_last_save_epoch_ms=" << m_headlessHostPresenceReport.lastPersistenceSaveEpochMilliseconds << "\n"
            << "host_persistence_last_load_epoch_ms=" << m_headlessHostPresenceReport.lastPersistenceLoadEpochMilliseconds << "\n"
            << "player_health=" << diagnostics.playerHealth << "\n"
            << "player_max_health=" << diagnostics.playerMaxHealth << "\n"
            << "player_armor=" << diagnostics.playerArmor << "\n"
            << "suit_integrity=" << diagnostics.suitIntegrity << "\n"
            << "oxygen_seconds=" << diagnostics.oxygenSecondsRemaining << "\n"
            << "radiation_dose=" << diagnostics.radiationDose << "\n"
            << "toxic_exposure=" << diagnostics.toxicExposure << "\n"
            << "current_hazard_label=" << sanitizeSingleLine(diagnostics.currentHazardLabel) << "\n"
            << "current_terrain_consequence=" << sanitizeSingleLine(diagnostics.currentTerrainConsequence) << "\n"
            << "inventory_stack_count=" << diagnostics.inventoryStackCount << "\n"
            << "inventory_item_count=" << diagnostics.inventoryItemCount << "\n"
            << "equipped_weapon=" << sanitizeSingleLine(diagnostics.equippedWeaponText) << "\n"
            << "equipped_suit=" << sanitizeSingleLine(diagnostics.equippedSuitText) << "\n"
            << "equipped_tool=" << sanitizeSingleLine(diagnostics.equippedToolText) << "\n"
            << "loot_collections=" << diagnostics.lootCollections << "\n"
            << "combat_active=" << (diagnostics.combatActive ? "yes" : "no") << "\n"
            << "combat_label=" << sanitizeSingleLine(diagnostics.currentCombatLabel) << "\n"
            << "combat_round=" << diagnostics.combatRoundNumber << "\n"
            << "combat_ticks_remaining=" << diagnostics.combatTicksRemaining << "\n"
            << "hostile_label=" << sanitizeSingleLine(diagnostics.hostileLabel) << "\n"
            << "hostile_health=" << diagnostics.hostileHealth << "\n"
            << "hostile_max_health=" << diagnostics.hostileMaxHealth << "\n"
            << "combat_rounds_resolved=" << diagnostics.combatRoundsResolved << "\n"
            << "encounter_wins=" << diagnostics.encounterWins << "\n"
            << "encounters_survived=" << diagnostics.encountersSurvived << "\n";

        const std::filesystem::path statusPath = m_runtimeBoundaryReport.logsDirectory / "client_replication_status.txt";
        if (!writeTextFileAtomically(statusPath, output.str()))
        {
            appendClientTrace(m_runtimeBoundaryReport, "GameLayer::writeClientReplicationStatus failed to publish status file");
        }
    }

    RECT GameLayer::getClientRect() const
    {
        RECT rect{};
        GetClientRect(m_window->getHandle(), &rect);
        return rect;
    }
}
