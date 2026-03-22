#pragma once

#include <array>
#include <memory>
#include <string>

#include <windows.h>

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/host/AuthoritativeHostProtocol.h"
#include "engine/host/HeadlessHostPresence.h"
#include "engine/host/ReplicationHarness.h"
#include "engine/host/SessionEntryProtocol.h"
#include "engine/math/Vec2.h"
#include "engine/render/BgfxDebugFrameRenderer.h"
#include "engine/render/BgfxWorldRenderer.h"
#include "engine/render/BgfxWorldTheme.h"
#include "engine/render/Camera2D.h"
#include "engine/render/DebugOverlayRenderer.h"
#include "engine/render/IRenderDevice.h"
#include "engine/render/WorldRenderer.h"
#include "engine/simulation/SimulationRuntime.h"
#include "platform/IWindow.h"

namespace war
{
    class GameLayer
    {
    public:
        void initialize(IWindow& window);
        void update(float dt);
        void render();
        void shutdown();

    private:
        void updateInput();
        void pushEvent(const std::string& message);
        void applyAuthoringHotkeys();
        void refreshAuthorityMode();
        void pollAuthoritativeHostResponses();
        void updateConnectionTelemetry();
        void updateSessionEntryFlow();
        void submitSessionEntryRequest(bool reconnectRequested);
        void loadPersistedResumeIdentity();
        void persistResumeIdentity() const;
        void loadPersistedLocationContext();
        void persistLocationContext() const;
        [[nodiscard]] bool tryResolveIssuedTicket(SessionTicket& outTicket) const;
        [[nodiscard]] bool tryResolveDeniedTicket(SessionTicket& outTicket) const;
        [[nodiscard]] bool hostConnectionCompatible(std::string& outReason) const;
        void persistReplicationHarnessConfig();
        void updateReplicationDiagnostics();
        void writeClientReplicationStatus() const;
        void updatePresentationRuntime();
        void handleCommandBarInput();
        void executeCommandLine(const std::string& commandLine);
        void submitTypedIntent(
            SimulationIntentType type,
            TileCoord target,
            const std::string& queueFailureMessage);
        [[nodiscard]] bool consumeKeyEdge(int virtualKey);
        void appendTypedCharacter(char character);
        [[nodiscard]] std::string buildRoomSignature() const;
        [[nodiscard]] std::string buildRoomTitle() const;
        [[nodiscard]] std::string buildRoomDescription() const;
        [[nodiscard]] std::string buildPromptLine() const;
        [[nodiscard]] std::string buildCommandBarText() const;

        [[nodiscard]] RECT getClientRect() const;

        IWindow* m_window = nullptr;
        Camera2D m_camera{};
        SimulationRuntime m_simulationRuntime{};
        RuntimeBoundaryReport m_runtimeBoundaryReport{};
        LocalDemoDiagnosticsReport m_localDemoDiagnosticsReport{};
        SessionEntryProtocolReport m_sessionEntryProtocolReport{};
        HeadlessHostPresenceReport m_headlessHostPresenceReport{};
        AuthoritativeHostProtocolReport m_authoritativeHostProtocolReport{};
        ReplicationHarnessConfig m_replicationHarnessConfig{};
        WorldRenderer m_worldRenderer{};
        DebugOverlayRenderer m_debugOverlayRenderer{};
        BgfxWorldRenderer m_bgfxWorldRenderer{};
        BgfxDebugFrameRenderer m_bgfxDebugFrameRenderer{};
        std::unique_ptr<IRenderDevice> m_renderDevice{};

        TileCoord m_hoveredTile{};
        bool m_hasHoveredTile = false;

        TileCoord m_selectedTile{};
        bool m_hasSelectedTile = false;

        TileCoord m_actionTargetTile{};
        bool m_hasActionTargetTile = false;

        float m_lastDeltaTime = 0.016f;
        uint64_t m_lastSnapshotAgeMilliseconds = 0;
        uint64_t m_lastAppliedSnapshotPublishedEpochMilliseconds = 0;
        uint64_t m_lastAppliedSnapshotSimulationTicks = 0;
        uint64_t m_lastAppliedSnapshotSequence = 0;
        uint64_t m_clientStartedEpochMilliseconds = 0;
        uint32_t m_expectedProtocolVersion = 2;
        bool m_useHeadlessHostAuthority = false;
        bool m_connectAttemptLogged = false;
        bool m_connectionEstablished = false;
        bool m_connectFailureLogged = false;
        std::string m_lastSnapshotReadError{};
        std::string m_clientInstanceId = "uninitialized";
        std::string m_clientSessionId = "uninitialized";
        std::string m_connectState = "uninitialized";
        std::string m_connectFailureReason = "none";
        std::string m_lastConnectEvent = "none";
        std::string m_lastDisconnectReason = "none";
        std::string m_lastHostSessionId = "none";
        std::string m_lastConnectedHostInstanceId = "none";

        std::string m_accountId = "internal-alpha-player";
        std::string m_playerIdentity = "operator-alpha";
        std::string m_sessionEntryState = "uninitialized";
        std::string m_sessionRequestId = "none";
        std::string m_sessionTicketId = "none";
        std::string m_sessionTicketState = "none";
        std::string m_sessionDenialReason = "none";
        std::string m_grantedSessionId = "none";
        std::string m_resumeSessionId = "none";
        uint64_t m_sessionTicketIssuedEpochMilliseconds = 0;
        uint64_t m_sessionTicketExpiresEpochMilliseconds = 0;
        bool m_sessionEntryRequestWritten = false;
        bool m_sessionTicketIssued = false;
        bool m_reconnectRequested = false;
        bool m_sessionDenialLogged = false;

        std::string m_roomSignature{};
        std::string m_roomTitle{};
        std::string m_roomDescription{};
        std::string m_promptLine{};
        std::string m_commandInput{};
        std::string m_commandEcho = "Type 'help' and press Enter.";
        std::array<bool, 256> m_keyWasDown{};

        bool m_overlayKeyWasDown = false;
        bool m_hotspotKeyWasDown = false;
        bool m_palette7WasDown = false;
        bool m_palette8WasDown = false;
        bool m_palette9WasDown = false;
        bool m_latencyToggleWasDown = false;
        bool m_latencyPresetWasDown = false;
        bool m_jitterPresetWasDown = false;
    };
}
