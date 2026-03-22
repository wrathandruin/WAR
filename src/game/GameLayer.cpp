#include "game/GameLayer.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

#include <windows.h>

#include "engine/host/HeadlessHostPresence.h"
#include "engine/render/BgfxRenderDevice.h"
#include "engine/render/GdiRenderDevice.h"

namespace war
{
    namespace
    {
        constexpr uint32_t kExpectedProtocolVersion = 2u;
        constexpr uint64_t kConnectFailureTimeoutMilliseconds = 2500ull;

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

        std::string buildClientIdentity(const char* prefix)
        {
            return std::string(prefix)
                + "-"
                + std::to_string(GetCurrentProcessId())
                + "-"
                + std::to_string(ReplicationHarness::currentEpochMilliseconds());
        }


std::filesystem::path clientResumeIdentityPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
{
    return runtimeBoundaryReport.configDirectory / "client_resume_identity.txt";
}

bool parseSimpleKeyValueFile(const std::filesystem::path& path, std::unordered_map<std::string, std::string>& outValues)
{
    outValues.clear();
    std::ifstream input(path, std::ios::in);
    if (!input.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(input, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        const size_t split = line.find('=');
        if (split == std::string::npos)
        {
            continue;
        }

        outValues.emplace(line.substr(0, split), line.substr(split + 1));
    }

    return true;
}

        std::string toLowerTrim(std::string value)
        {
            value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }));
            value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }).base(), value.end());

            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
            {
                return static_cast<char>(std::tolower(ch));
            });

            return value;
        }

        const char* regionTitle(WorldRegionTagId tag)
        {
            switch (tag)
            {
            case WorldRegionTagId::CargoBay: return "Khepri Dock Cargo Bay";
            case WorldRegionTagId::TransitSpine: return "Transit Spine";
            case WorldRegionTagId::MedLab: return "MedLab Diagnostics";
            case WorldRegionTagId::CommandDeck: return "Command Deck Approach";
            case WorldRegionTagId::HazardContainment: return "Hazard Containment";
            default: return "Unknown Interior";
            }
        }

        const char* regionDescription(WorldRegionTagId tag)
        {
            switch (tag)
            {
            case WorldRegionTagId::CargoBay:
                return "Cargo lifters sit dark under pressure lamps while the docked responder shuttle waits in its collar.";
            case WorldRegionTagId::TransitSpine:
                return "A narrow industrial transit corridor channels personnel and freight through the station's central spine.";
            case WorldRegionTagId::MedLab:
                return "Sterile light, diagnostic trays, and sealed med stations make this bay feel colder than the rest of the dock.";
            case WorldRegionTagId::CommandDeck:
                return "The command lane narrows into a defended choke where control authority and quarantine routing become decisive.";
            case WorldRegionTagId::HazardContainment:
                return "Containment walls are scarred by leak alarms and emergency patches; the air here feels recently fought over.";
            default:
                return "The current interior has not been formally authored yet.";
            }
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
        m_localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(m_runtimeBoundaryReport);
        LocalDemoDiagnostics::writeStartupReport(m_runtimeBoundaryReport, m_localDemoDiagnosticsReport);
        m_headlessHostPresenceReport = HeadlessHostPresence::buildReport(m_runtimeBoundaryReport);
        m_authoritativeHostProtocolReport = AuthoritativeHostProtocol::buildReport(m_runtimeBoundaryReport);
        m_replicationHarnessConfig = ReplicationHarness::loadConfig(m_runtimeBoundaryReport);
        m_sessionEntryProtocolReport = SessionEntryProtocol::buildReport(m_runtimeBoundaryReport);

        m_expectedProtocolVersion = kExpectedProtocolVersion;
        m_clientStartedEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        m_clientInstanceId = buildClientIdentity("client");
        m_clientSessionId = buildClientIdentity("client-session");
        m_connectState = "connect-pending";
        m_connectFailureReason = "none";
        loadPersistedResumeIdentity();
        updateSessionEntryFlow();

        refreshAuthorityMode();
        updateConnectionTelemetry();
        updateReplicationDiagnostics();
        updatePresentationRuntime();
        writeClientReplicationStatus();

        pushEvent("Milestone 47 initialized");
        pushEvent("account session ticket handoff / authenticated entry active");
        pushEvent(std::string("Client instance: ") + m_clientInstanceId);
        pushEvent(std::string("Client session: ") + m_clientSessionId);
        pushEvent(std::string("Connect target: ") + m_localDemoDiagnosticsReport.connectTargetName);
        pushEvent(std::string("Transport: ") + m_localDemoDiagnosticsReport.connectTransport);
        pushEvent(std::string("Lane mode: ") + m_localDemoDiagnosticsReport.connectLaneMode);
        pushEvent(std::string("Session account: ") + m_accountId);
        pushEvent(std::string("Session identity: ") + m_playerIdentity);
        pushEvent("MUD surfaces live: room descriptions, prompt strip, and typed command shell are active.");
        if (m_resumeSessionId != "none")
        {
            pushEvent(std::string("Resume identity detected: ") + m_resumeSessionId);
        }
        pushEvent("Type 'help' and press Enter for MVP shell commands.");

        auto preferred = std::make_unique<BgfxRenderDevice>();
        if (preferred->initialize(m_window->getHandle()))
        {
            m_renderDevice = std::move(preferred);
            return;
        }

        auto fallback = std::make_unique<GdiRenderDevice>();
        fallback->initialize(m_window->getHandle());
        m_renderDevice = std::move(fallback);
    }

    void GameLayer::update(float dt)
    {
        m_lastDeltaTime = dt;
        m_camera.setViewportSize(m_window->getWidth(), m_window->getHeight());

        m_headlessHostPresenceReport = HeadlessHostPresence::buildReport(m_runtimeBoundaryReport);
        m_authoritativeHostProtocolReport = AuthoritativeHostProtocol::buildReport(m_runtimeBoundaryReport);
        updateSessionEntryFlow();
        refreshAuthorityMode();
        updateConnectionTelemetry();
        pollAuthoritativeHostResponses();
        updateReplicationDiagnostics();
        updateInput();
        m_simulationRuntime.advanceFrame(dt);
        updatePresentationRuntime();

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
                m_authoritativeHostProtocolReport,
                m_roomTitle,
                m_roomDescription,
                m_promptLine,
                buildCommandBarText(),
                m_commandEcho);
        }
        else
        {
            (void)m_bgfxWorldRenderer.render(
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

            m_bgfxDebugFrameRenderer.render(
                worldState,
                playerPosition,
                eventLog,
                m_lastDeltaTime,
                m_bgfxWorldRenderer.statusMessage(),
                simulationDiagnostics,
                m_roomTitle,
                m_promptLine,
                buildCommandBarText());
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
        handleCommandBarInput();

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

            submitTypedIntent(SimulationIntentType::MoveToTile, targetTile, "Failed to queue move intent.");
        }

        POINT rightClick{};
        if (m_window->consumeRightClick(rightClick))
        {
            const Vec2 world = m_camera.screenToWorld(rightClick.x, rightClick.y);
            const TileCoord targetTile = worldState.world().worldToTile(world);

            m_selectedTile = targetTile;
            m_hasSelectedTile = worldState.world().isInBounds(targetTile);

            const bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            submitTypedIntent(
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
        std::string compatibilityReason;
        const bool hostCompatible = hostConnectionCompatible(compatibilityReason);
        const bool sessionGranted = m_sessionTicketIssued && m_sessionEntryState == "ticket-issued";
        const bool previousMode = m_useHeadlessHostAuthority;
        m_useHeadlessHostAuthority = hostCompatible && sessionGranted;

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
            pushEvent(std::string("Authority mode: ticket accepted for session ") + m_grantedSessionId + ", hosted headless host authority active");
            return;
        }

        if (hostCompatible && !sessionGranted)
        {
            pushEvent("Authority mode: host online but session-entry ticket not yet granted");
            return;
        }

        pushEvent(std::string("Authority mode: local fallback active (") + compatibilityReason + ")");
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


    void GameLayer::updateSessionEntryFlow()
    {
        m_sessionEntryProtocolReport = SessionEntryProtocol::buildReport(m_runtimeBoundaryReport);

        if (!m_sessionEntryRequestWritten && m_sessionEntryProtocolReport.sessionEntryLaneReady)
        {
            submitSessionEntryRequest(m_reconnectRequested);
        }

        if (!m_sessionEntryRequestWritten)
        {
            m_sessionEntryState = m_sessionEntryProtocolReport.sessionEntryLaneReady
                ? "entry-request-pending-write"
                : "entry-lane-unavailable";
            return;
        }

        SessionTicket deniedTicket{};
        if (!m_sessionTicketIssued && tryResolveDeniedTicket(deniedTicket))
        {
            m_sessionTicketId = deniedTicket.ticketId;
            m_sessionTicketState = "denied";
            m_sessionDenialReason = deniedTicket.denialReason;
            m_sessionEntryState = "entry-denied";
            m_sessionTicketIssued = false;
            if (!m_sessionDenialLogged)
            {
                pushEvent(std::string("Session entry denied: ") + m_sessionDenialReason);
                m_commandEcho = std::string("Entry denied: ") + m_sessionDenialReason + ". Type 'entry' for a fresh request or 'resume' to retry resume identity.";
                m_sessionDenialLogged = true;
            }
            return;
        }

        SessionTicket issuedTicket{};
        if (tryResolveIssuedTicket(issuedTicket))
        {
            const bool firstIssue = !m_sessionTicketIssued || m_sessionTicketId != issuedTicket.ticketId;
            m_sessionTicketIssued = true;
            m_sessionTicketId = issuedTicket.ticketId;
            m_sessionTicketState = "issued";
            m_sessionDenialReason = "none";
            m_grantedSessionId = issuedTicket.grantedSessionId;
            m_resumeSessionId = issuedTicket.grantedSessionId;
            m_sessionTicketIssuedEpochMilliseconds = issuedTicket.issuedAtEpochMilliseconds;
            m_sessionTicketExpiresEpochMilliseconds = issuedTicket.expiresAtEpochMilliseconds;
            m_sessionEntryState = "ticket-issued";
            if (firstIssue)
            {
                persistResumeIdentity();
                pushEvent(std::string("Session ticket issued: ") + m_sessionTicketId);
                pushEvent(std::string("Granted session identity: ") + m_grantedSessionId);
                m_commandEcho = std::string("Entry granted for session ") + m_grantedSessionId + ".";
            }
            return;
        }

        if (m_sessionEntryState != "ticket-issued")
        {
            m_sessionEntryState = m_reconnectRequested ? "entry-pending-resume-ticket" : "entry-pending-ticket";
        }
    }

    void GameLayer::submitSessionEntryRequest(bool reconnectRequested)
    {
        if (!m_sessionEntryProtocolReport.sessionEntryLaneReady)
        {
            return;
        }

        SessionEntryRequest request{};
        request.requestId = std::string("request-") + m_clientInstanceId + "-" + std::to_string(ReplicationHarness::currentEpochMilliseconds());
        request.accountId = m_accountId;
        request.playerIdentity = m_playerIdentity;
        request.clientInstanceId = m_clientInstanceId;
        request.buildIdentity = m_localDemoDiagnosticsReport.buildIdentity;
        request.environmentName = m_localDemoDiagnosticsReport.environmentName;
        request.connectTargetName = m_localDemoDiagnosticsReport.connectTargetName;
        request.requestedResumeSessionId = reconnectRequested ? m_resumeSessionId : std::string("none");
        request.requestedAtEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        request.requestedTicketTtlSeconds = 120u;
        request.reconnectRequested = reconnectRequested;

        std::string error;
        if (!SessionEntryProtocol::writeEntryRequest(m_runtimeBoundaryReport, request, error))
        {
            pushEvent(std::string("Failed to write session entry request: ") + error);
            m_sessionEntryState = "entry-request-write-failed";
            m_sessionDenialReason = error;
            return;
        }

        m_sessionEntryRequestWritten = true;
        m_sessionTicketIssued = false;
        m_reconnectRequested = reconnectRequested;
        m_sessionDenialLogged = false;
        m_sessionRequestId = request.requestId;
        m_sessionTicketId = "none";
        m_sessionTicketState = "none";
        m_sessionDenialReason = "none";
        m_grantedSessionId = "none";
        m_sessionEntryState = reconnectRequested ? "entry-requested-resume" : "entry-requested";
        pushEvent(std::string("Session entry request submitted: ") + m_sessionRequestId);
        if (reconnectRequested)
        {
            pushEvent(std::string("Reconnect requested with resume session ") + m_resumeSessionId);
        }
    }

    void GameLayer::loadPersistedResumeIdentity()
    {
        m_resumeSessionId = "none";
        std::unordered_map<std::string, std::string> values{};
        if (!parseSimpleKeyValueFile(clientResumeIdentityPath(m_runtimeBoundaryReport), values))
        {
            return;
        }

        const auto sessionIt = values.find("granted_session_id");
        if (sessionIt != values.end() && !sessionIt->second.empty())
        {
            m_resumeSessionId = sessionIt->second;
        }

        const auto accountIt = values.find("account_id");
        if (accountIt != values.end() && !accountIt->second.empty())
        {
            m_accountId = accountIt->second;
        }

        const auto playerIt = values.find("player_identity");
        if (playerIt != values.end() && !playerIt->second.empty())
        {
            m_playerIdentity = playerIt->second;
        }
    }

    void GameLayer::persistResumeIdentity() const
    {
        if (m_grantedSessionId == "none")
        {
            return;
        }

        std::ostringstream output;
        output
            << "granted_session_id=" << sanitizeSingleLine(m_grantedSessionId) << "\n"
            << "account_id=" << sanitizeSingleLine(m_accountId) << "\n"
            << "player_identity=" << sanitizeSingleLine(m_playerIdentity) << "\n"
            << "ticket_id=" << sanitizeSingleLine(m_sessionTicketId) << "\n";
        writeTextFileAtomically(clientResumeIdentityPath(m_runtimeBoundaryReport), output.str());
    }

    bool GameLayer::tryResolveIssuedTicket(SessionTicket& outTicket) const
    {
        const std::vector<SessionTicket> tickets = SessionEntryProtocol::collectIssuedTickets(m_runtimeBoundaryReport);
        for (auto it = tickets.rbegin(); it != tickets.rend(); ++it)
        {
            if (it->requestId == SessionEntryProtocol::sanitizeIdentifier(m_sessionRequestId)
                && it->clientInstanceId == SessionEntryProtocol::sanitizeIdentifier(m_clientInstanceId))
            {
                outTicket = *it;
                return true;
            }
        }

        return false;
    }

    bool GameLayer::tryResolveDeniedTicket(SessionTicket& outTicket) const
    {
        const std::vector<SessionTicket> tickets = SessionEntryProtocol::collectDeniedTickets(m_runtimeBoundaryReport);
        for (auto it = tickets.rbegin(); it != tickets.rend(); ++it)
        {
            if (it->requestId == SessionEntryProtocol::sanitizeIdentifier(m_sessionRequestId)
                && it->clientInstanceId == SessionEntryProtocol::sanitizeIdentifier(m_clientInstanceId))
            {
                outTicket = *it;
                return true;
            }
        }

        return false;
    }


    void GameLayer::updateConnectionTelemetry()
    {
        if (!m_connectAttemptLogged)
        {
            const std::string attemptMessage = std::string("Connect attempt: target=")
                + m_localDemoDiagnosticsReport.connectTargetName
                + " | transport="
                + m_localDemoDiagnosticsReport.connectTransport
                + " | lane="
                + m_localDemoDiagnosticsReport.connectLaneMode
                + " | runtime_root="
                + m_localDemoDiagnosticsReport.runtimeRootDisplay;
            pushEvent(attemptMessage);
            appendClientTrace(m_runtimeBoundaryReport, attemptMessage);
            m_lastConnectEvent = "attempt";
            m_connectAttemptLogged = true;
        }

        std::string compatibilityReason;
        const bool hostCompatible = hostConnectionCompatible(compatibilityReason);
        const bool sessionGranted = m_sessionTicketIssued && m_sessionEntryState == "ticket-issued";

        if (hostCompatible && sessionGranted)
        {
            const bool sessionChanged = !m_connectionEstablished
                || m_lastHostSessionId != m_headlessHostPresenceReport.sessionId
                || m_lastConnectedHostInstanceId != m_headlessHostPresenceReport.hostInstanceId;

            m_connectState = "connected-headless-host";
            m_connectFailureReason = "none";
            m_connectionEstablished = true;
            m_connectFailureLogged = false;

            if (sessionChanged)
            {
                const std::string connectedMessage = std::string("Connect success: target=")
                    + m_headlessHostPresenceReport.connectTargetName
                    + " | transport="
                    + m_headlessHostPresenceReport.transportKind
                    + " | host_session="
                    + m_headlessHostPresenceReport.sessionId
                    + " | granted_session="
                    + m_grantedSessionId
                    + " | host_instance="
                    + m_headlessHostPresenceReport.hostInstanceId
                    + " | protocol=v"
                    + std::to_string(m_headlessHostPresenceReport.protocolVersion)
                    + " | build="
                    + m_headlessHostPresenceReport.buildIdentity;
                pushEvent(connectedMessage);
                appendClientTrace(m_runtimeBoundaryReport, connectedMessage);
                m_lastConnectEvent = "connected";
                m_lastHostSessionId = m_headlessHostPresenceReport.sessionId;
                m_lastConnectedHostInstanceId = m_headlessHostPresenceReport.hostInstanceId;
            }
            return;
        }

        if (hostCompatible && !sessionGranted)
        {
            m_connectState = m_reconnectRequested ? "entry-pending-resume-ticket" : "entry-pending-ticket";
            m_connectFailureReason = "none";
            m_connectionEstablished = false;
            return;
        }

        const uint64_t nowEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        const uint64_t elapsedMilliseconds = nowEpochMilliseconds >= m_clientStartedEpochMilliseconds
            ? nowEpochMilliseconds - m_clientStartedEpochMilliseconds
            : 0;

        if (m_connectionEstablished)
        {
            const std::string disconnectMessage = std::string("Disconnect event: reason=")
                + compatibilityReason
                + " | target="
                + m_localDemoDiagnosticsReport.connectTargetName
                + " | falling back to local authority";
            pushEvent(disconnectMessage);
            appendClientTrace(m_runtimeBoundaryReport, disconnectMessage);
            m_connectionEstablished = false;
            m_connectState = "fallback-local";
            m_connectFailureReason = compatibilityReason;
            m_lastDisconnectReason = compatibilityReason;
            m_lastConnectEvent = "disconnected";
            return;
        }

        if (m_sessionEntryState == "entry-denied")
        {
            m_connectState = "entry-denied";
            m_connectFailureReason = m_sessionDenialReason;
            m_lastConnectEvent = "denied";
            return;
        }

        if (elapsedMilliseconds < kConnectFailureTimeoutMilliseconds)
        {
            m_connectState = "connect-pending";
            m_connectFailureReason = "none";
            return;
        }

        m_connectState = "fallback-local";
        m_connectFailureReason = compatibilityReason.empty() ? std::string("host-offline") : compatibilityReason;
        if (!m_connectFailureLogged)
        {
            std::ostringstream failureMessage;
            failureMessage
                << "Connect failure: target=" << m_localDemoDiagnosticsReport.connectTargetName
                << " unavailable (" << m_connectFailureReason << ")"
                << " | transport=" << m_localDemoDiagnosticsReport.connectTransport
                << " | lane=" << m_localDemoDiagnosticsReport.connectLaneMode
                << " | runtime_root=" << m_localDemoDiagnosticsReport.runtimeRootDisplay;

            pushEvent(failureMessage.str());
            appendClientTrace(m_runtimeBoundaryReport, failureMessage.str());
            m_lastConnectEvent = "failed";
            m_connectFailureLogged = true;
        }
    }

    bool GameLayer::hostConnectionCompatible(std::string& outReason) const
    {
        outReason.clear();

        if (!m_headlessHostPresenceReport.statusFilePresent)
        {
            outReason = "host-status-missing";
            return false;
        }

        if (!m_headlessHostPresenceReport.statusParseValid)
        {
            outReason = "host-status-invalid";
            return false;
        }

        if (!m_headlessHostPresenceReport.heartbeatFresh || !m_headlessHostPresenceReport.hostOnline)
        {
            outReason = "host-offline";
            return false;
        }

        if (m_headlessHostPresenceReport.protocolVersion != m_expectedProtocolVersion)
        {
            outReason = "protocol-version-mismatch";
            return false;
        }

        if (m_headlessHostPresenceReport.transportKind != m_localDemoDiagnosticsReport.connectTransport)
        {
            outReason = "transport-mismatch";
            return false;
        }

        if (m_headlessHostPresenceReport.connectTargetName != m_localDemoDiagnosticsReport.connectTargetName)
        {
            outReason = "connect-target-mismatch";
            return false;
        }

        return true;
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
            << "version=6\n"
            << "build_identity=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.buildIdentity) << "\n"
            << "build_channel=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.buildChannel) << "\n"
            << "client_instance_id=" << sanitizeSingleLine(m_clientInstanceId) << "\n"
            << "client_session_id=" << sanitizeSingleLine(m_clientSessionId) << "\n"
            << "connect_target_name=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.connectTargetName) << "\n"
            << "connect_transport=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.connectTransport) << "\n"
            << "connect_lane_mode=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.connectLaneMode) << "\n"
            << "runtime_root=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.runtimeRootDisplay) << "\n"
            << "connect_state=" << sanitizeSingleLine(m_connectState) << "\n"
            << "connect_failure_reason=" << sanitizeSingleLine(m_connectFailureReason) << "\n"
            << "last_connect_event=" << sanitizeSingleLine(m_lastConnectEvent) << "\n"
            << "last_disconnect_reason=" << sanitizeSingleLine(m_lastDisconnectReason) << "\n"
            << "session_entry_state=" << sanitizeSingleLine(m_sessionEntryState) << "\n"
            << "session_request_id=" << sanitizeSingleLine(m_sessionRequestId) << "\n"
            << "session_ticket_id=" << sanitizeSingleLine(m_sessionTicketId) << "\n"
            << "session_ticket_state=" << sanitizeSingleLine(m_sessionTicketState) << "\n"
            << "session_denial_reason=" << sanitizeSingleLine(m_sessionDenialReason) << "\n"
            << "granted_session_id=" << sanitizeSingleLine(m_grantedSessionId) << "\n"
            << "resume_session_id=" << sanitizeSingleLine(m_resumeSessionId) << "\n"
            << "reconnect_requested=" << (m_reconnectRequested ? "yes" : "no") << "\n"
            << "account_id=" << sanitizeSingleLine(m_accountId) << "\n"
            << "player_identity=" << sanitizeSingleLine(m_playerIdentity) << "\n"
            << "expected_protocol_version=" << m_expectedProtocolVersion << "\n"
            << "authority_mode=" << (m_useHeadlessHostAuthority ? "headless-host" : "local") << "\n"
            << "host_online=" << (m_headlessHostPresenceReport.hostOnline ? "yes" : "no") << "\n"
            << "host_protocol_version=" << m_headlessHostPresenceReport.protocolVersion << "\n"
            << "host_transport_kind=" << sanitizeSingleLine(m_headlessHostPresenceReport.transportKind) << "\n"
            << "host_connect_target_name=" << sanitizeSingleLine(m_headlessHostPresenceReport.connectTargetName) << "\n"
            << "host_connect_lane_mode=" << sanitizeSingleLine(m_headlessHostPresenceReport.connectLaneMode) << "\n"
            << "host_build_identity=" << sanitizeSingleLine(m_headlessHostPresenceReport.buildIdentity) << "\n"
            << "host_instance_id=" << sanitizeSingleLine(m_headlessHostPresenceReport.hostInstanceId) << "\n"
            << "host_session_id=" << sanitizeSingleLine(m_headlessHostPresenceReport.sessionId) << "\n"
            << "host_restore_state=" << sanitizeSingleLine(m_headlessHostPresenceReport.restoreState) << "\n"
            << "host_persistence_last_load_succeeded=" << (m_headlessHostPresenceReport.persistenceLastLoadSucceeded ? "yes" : "no") << "\n"
            << "protocol_lane_ready=" << (m_authoritativeHostProtocolReport.authorityLaneReady ? "yes" : "no") << "\n"
            << "snapshot_present=" << (m_authoritativeHostProtocolReport.snapshotPresent ? "yes" : "no") << "\n"
            << "room_title=" << sanitizeSingleLine(m_roomTitle) << "\n"
            << "room_description=" << sanitizeSingleLine(m_roomDescription) << "\n"
            << "prompt_line=" << sanitizeSingleLine(m_promptLine) << "\n"
            << "command_bar=" << sanitizeSingleLine(buildCommandBarText()) << "\n"
            << "command_echo=" << sanitizeSingleLine(m_commandEcho) << "\n"
            << "client_prediction_enabled=" << (diagnostics.clientPredictionEnabled ? "yes" : "no") << "\n";

        const std::filesystem::path statusPath = m_runtimeBoundaryReport.logsDirectory / "client_replication_status.txt";
        writeTextFileAtomically(statusPath, output.str());
    }

    void GameLayer::updatePresentationRuntime()
    {
        m_roomTitle = buildRoomTitle();
        m_roomDescription = buildRoomDescription();
        m_promptLine = buildPromptLine();

        const std::string newSignature = buildRoomSignature();
        if (!newSignature.empty() && newSignature != m_roomSignature)
        {
            m_roomSignature = newSignature;
            pushEvent(std::string("Room entry: ") + m_roomTitle);
            pushEvent(m_roomDescription);
        }
    }

    void GameLayer::handleCommandBarInput()
    {
        if (consumeKeyEdge(VK_BACK))
        {
            if (!m_commandInput.empty())
            {
                m_commandInput.pop_back();
            }
        }

        if (consumeKeyEdge(VK_SPACE))
        {
            appendTypedCharacter(' ');
        }

        for (int key = 'A'; key <= 'Z'; ++key)
        {
            if (consumeKeyEdge(key))
            {
                appendTypedCharacter(static_cast<char>(std::tolower(key)));
            }
        }

        for (int key = '0'; key <= '9'; ++key)
        {
            if (consumeKeyEdge(key))
            {
                appendTypedCharacter(static_cast<char>(key));
            }
        }

        if (consumeKeyEdge(VK_RETURN))
        {
            const std::string submitted = m_commandInput;
            m_commandInput.clear();
            executeCommandLine(submitted);
        }
    }

    void GameLayer::executeCommandLine(const std::string& commandLine)
    {
        const std::string normalized = toLowerTrim(commandLine);
        if (normalized.empty())
        {
            m_commandEcho = "No command entered.";
            return;
        }

        if (normalized == "help")
        {
            m_commandEcho = "Commands: help, look, room, where, status, vitals, session, entry, resume, inspect, interact, move x y, clear.";
            return;
        }

        if (normalized == "look" || normalized == "room" || normalized == "where")
        {
            m_commandEcho = std::string("Displayed room text for ") + m_roomTitle + ".";
            pushEvent(std::string("Room: ") + m_roomTitle);
            pushEvent(m_roomDescription);
            return;
        }

        if (normalized == "status" || normalized == "vitals")
        {
            m_commandEcho = "Displayed vitals prompt.";
            pushEvent(std::string("Vitals: ") + m_promptLine);
            return;
        }

        if (normalized == "session")
        {
            m_commandEcho = std::string("Entry state: ") + m_sessionEntryState + ", granted session: " + m_grantedSessionId + ", resume identity: " + m_resumeSessionId + ".";
            pushEvent(std::string("Session entry: ") + m_sessionEntryState + " | granted=" + m_grantedSessionId + " | resume=" + m_resumeSessionId);
            return;
        }

        if (normalized == "entry")
        {
            m_sessionEntryRequestWritten = false;
            m_sessionTicketIssued = false;
            m_reconnectRequested = false;
            submitSessionEntryRequest(false);
            m_commandEcho = "Fresh session entry requested.";
            return;
        }

        if (normalized == "resume")
        {
            if (m_resumeSessionId == "none")
            {
                m_commandEcho = "No persisted resume session is available yet.";
                return;
            }

            m_sessionEntryRequestWritten = false;
            m_sessionTicketIssued = false;
            m_reconnectRequested = true;
            submitSessionEntryRequest(true);
            m_commandEcho = std::string("Reconnect requested for resume session ") + m_resumeSessionId + ".";
            return;
        }

        if (normalized == "inspect")
        {
            const TileCoord target = m_hasSelectedTile
                ? m_selectedTile
                : m_simulationRuntime.worldState().world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());
            submitTypedIntent(SimulationIntentType::InspectTile, target, "Failed to queue inspect intent.");
            m_commandEcho = "Inspect queued.";
            return;
        }

        if (normalized == "interact")
        {
            const TileCoord target = m_hasSelectedTile
                ? m_selectedTile
                : m_simulationRuntime.worldState().world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());
            submitTypedIntent(SimulationIntentType::InteractTile, target, "Failed to queue interact intent.");
            m_commandEcho = "Interact queued.";
            return;
        }

        if (normalized == "clear")
        {
            m_commandEcho = "Command reply cleared.";
            return;
        }

        if (normalized.rfind("move ", 0) == 0)
        {
            std::istringstream input(normalized.substr(5));
            int x = 0;
            int y = 0;
            if (input >> x >> y)
            {
                TileCoord target{ x, y };
                m_selectedTile = target;
                m_hasSelectedTile = true;
                m_actionTargetTile = target;
                m_hasActionTargetTile = true;
                submitTypedIntent(SimulationIntentType::MoveToTile, target, "Failed to queue move intent.");
                m_commandEcho = "Move queued.";
                return;
            }
            m_commandEcho = "Move syntax: move <x> <y>";
            return;
        }

        m_commandEcho = std::string("Unknown command: ") + normalized + ". Type 'help'.";
    }

    void GameLayer::submitTypedIntent(SimulationIntentType type, TileCoord target, const std::string& queueFailureMessage)
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
        pushEvent(std::string("Failed to submit intent to headless host: ") + protocolError);
    }

    bool GameLayer::consumeKeyEdge(int virtualKey)
    {
        if (virtualKey < 0 || virtualKey >= static_cast<int>(m_keyWasDown.size()))
        {
            return false;
        }

        const bool down = (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
        const bool pressed = down && !m_keyWasDown[static_cast<size_t>(virtualKey)];
        m_keyWasDown[static_cast<size_t>(virtualKey)] = down;
        return pressed;
    }

    void GameLayer::appendTypedCharacter(char character)
    {
        if (m_commandInput.size() < 64u)
        {
            m_commandInput.push_back(character);
        }
    }

    std::string GameLayer::buildRoomSignature() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const WorldState& worldState = m_simulationRuntime.worldState();
        const TileCoord playerTile = worldState.world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());
        std::ostringstream signature;
        signature << diagnostics.playerRuntimeContextText << '|' << diagnostics.missionPhaseText << '|' << playerTile.x << ',' << playerTile.y;
        return signature.str();
    }

    std::string GameLayer::buildRoomTitle() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const WorldState& worldState = m_simulationRuntime.worldState();
        const TileCoord playerTile = worldState.world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());

        if (diagnostics.orbitalLayerActive)
        {
            return "Orbital Approach";
        }

        if (diagnostics.shipBoarded)
        {
            return diagnostics.shipCommandClaimed
                ? "Responder Shuttle Khepri - Command Cabin"
                : "Responder Shuttle Khepri - Docked Interior";
        }

        if (diagnostics.frontierSurfaceActive)
        {
            return "Dust Frontier Landing Pad";
        }

        if (const WorldAuthoringHotspot* hotspot = worldState.authoringHotspotAt(playerTile))
        {
            if (!hotspot->label.empty())
            {
                return hotspot->label;
            }
        }

        return regionTitle(worldState.regionTag(playerTile));
    }

    std::string GameLayer::buildRoomDescription() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const WorldState& worldState = m_simulationRuntime.worldState();
        const TileCoord playerTile = worldState.world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());

        std::ostringstream description;
        if (diagnostics.orbitalLayerActive)
        {
            description << "The shuttle sits inside an interim hosted-bootstrap orbital lane with route logic surfaced through the current navigation phase.";
            if (!diagnostics.orbitalRuleText.empty())
            {
                description << ' ' << diagnostics.orbitalRuleText;
            }
            return description.str();
        }

        if (diagnostics.shipBoarded)
        {
            description << "A cramped responder-shuttle cabin wraps the player in steel, harness webbing, helm hardware, and a navigation console.";
            return description.str();
        }

        if (diagnostics.frontierSurfaceActive)
        {
            description << "Dust Frontier's landing pad sits under hard vacuum glare and relay dust, with the return shuttle acting as the only credible way back.";
            return description.str();
        }

        description << regionDescription(worldState.regionTag(playerTile));
        if (const WorldAuthoringHotspot* hotspot = worldState.authoringHotspotAt(playerTile))
        {
            if (!hotspot->summary.empty())
            {
                description << ' ' << hotspot->summary;
            }
        }
        description << " Current objective: " << diagnostics.missionObjectiveText;
        return description.str();
    }

    std::string GameLayer::buildPromptLine() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        std::ostringstream prompt;
        prompt
            << "HP " << diagnostics.playerHealth << "/" << diagnostics.playerMaxHealth
            << " | ARM " << diagnostics.playerArmor
            << " | O2 " << static_cast<int>(diagnostics.oxygenSecondsRemaining) << "s"
            << " | PHASE " << diagnostics.missionPhaseText
            << " | ENTRY " << m_sessionEntryState;
        return prompt.str();
    }

    std::string GameLayer::buildCommandBarText() const
    {
        return std::string("> ") + m_commandInput + '_';
    }

    RECT GameLayer::getClientRect() const
    {
        RECT rect{};
        GetClientRect(m_window->getHandle(), &rect);
        return rect;
    }
}
