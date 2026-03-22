#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <windows.h>

#include "engine/content/SessionMvpAuthoring.h"
#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/host/AuthoritativeHostProtocol.h"
#include "engine/host/HeadlessHostPresence.h"
#include "engine/host/LocalPresenceProtocol.h"
#include "engine/host/LocalSocialProtocol.h"
#include "engine/host/ReplicationHarness.h"
#include "engine/host/ReconnectSessionProtocol.h"
#include "engine/host/SessionEntryProtocol.h"
#include "engine/math/Vec2.h"
#include "engine/render/BgfxDebugFrameRenderer.h"
#include "engine/render/RemotePresenceRenderProxy.h"
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
        void loadPersistedSocialState();
        void persistSocialState() const;
        void loadPersistedReconnectContinuity();
        void persistReconnectContinuity();
        void loadSessionMvpAuthoring();
        void updateReconnectRecoveryRuntime();
        [[nodiscard]] std::string buildReconnectContinuitySummary() const;
        [[nodiscard]] std::string buildMissionObjectiveText() const;
        [[nodiscard]] std::string buildMissionJournalText() const;
        void updateLocalSocialRuntime();
        void updateLocalPresenceRuntime();
        void routeLocalSocialCommand(LocalSocialMessageKind kind, const std::string& text);
        [[nodiscard]] std::string buildSocialParticipantId() const;
        [[nodiscard]] bool tryResolveIssuedTicket(SessionTicket& outTicket) const;
        [[nodiscard]] bool tryResolveDeniedTicket(SessionTicket& outTicket) const;
        [[nodiscard]] bool hostConnectionCompatible(std::string& outReason) const;
        void initializeSessionIdentityDefaults();
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
        [[nodiscard]] std::string buildEnvironmentIdentityText(const std::string& roomSignature) const;
        [[nodiscard]] std::string buildTextPresentationProfile(const std::string& roomSignature) const;

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
        std::string m_clientIdentitySlot = "alpha";
        std::filesystem::path m_clientIdentityClaimPath{};

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
        std::string m_environmentIdentityText = "Khepri Industrial Dock";
        std::string m_textPresentationProfile = "industrial-manifest-shell";
        std::string m_artManifestPath = "Content/SessionMvp/session_mvp_art_manifest.txt";
        std::string m_commandInput{};
        std::string m_commandEcho = "Type 'help' and press Enter.";
        std::array<bool, 256> m_keyWasDown{};

        uint64_t m_lastSeenSocialEpochMilliseconds = 0;
        std::string m_socialLocationKey = "none";
        std::string m_socialParticipantId = "none";
        std::string m_lastSocialPollError{};

        uint64_t m_lastPresenceHeartbeatEpochMilliseconds = 0;
        uint64_t m_lastPresenceRosterEpochMilliseconds = 0;
        uint32_t m_sameSpacePresenceCount = 0;
        std::string m_presenceLocationKey = "none";
        std::string m_sameSpacePresenceSummary = "none";
        std::string m_lastPresencePollError{};
        std::vector<std::string> m_visiblePresenceParticipantIds{};
        std::vector<RemotePresenceRenderProxy> m_visiblePresenceRenderProxies{};
        struct RemotePresenceTrack
        {
            uint64_t previousEpochMilliseconds = 0;
            uint64_t latestEpochMilliseconds = 0;
            Vec2 previousWorldPosition{ 0.0f, 0.0f };
            Vec2 latestWorldPosition{ 0.0f, 0.0f };
            Vec2 presentedWorldPosition{ 0.0f, 0.0f };
        };
        std::unordered_map<std::string, RemotePresenceTrack> m_remotePresenceTracks{};
        Vec2 m_lastPresenceHeartbeatWorldPosition{ 0.0f, 0.0f };
        bool m_hasLastPresenceHeartbeatWorldPosition = false;

        SessionContinuitySnapshot m_reconnectContinuitySnapshot{};
        uint64_t m_lastReconnectLeaseEpochMilliseconds = 0;
        bool m_reconnectContinuityRestored = false;
        uint32_t m_reconnectStaleSessionCount = 0;
        std::string m_reconnectRecoveryState = "cold-start";
        std::string m_reconnectRecoveryReason = "none";
        std::string m_reconnectContinuitySummary = "none";
        std::string m_lastReconnectPollError{};

        SessionMvpAuthoringBundle m_sessionMvpAuthoring{};
        bool m_sessionMvpAuthoringLoaded = false;
        std::string m_sessionMvpAuthoringError = "none";
        std::string m_sessionMvpAuthoringSource = "none";
        std::string m_missionObjectiveText{};
        std::string m_missionJournalText{};
        std::string m_lastMissionJournalPhaseKey = "none";

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
