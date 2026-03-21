#pragma once

#include <memory>
#include <string>

#include <windows.h>

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/host/AuthoritativeHostProtocol.h"
#include "engine/host/HeadlessHostPresence.h"
#include "engine/host/ReplicationHarness.h"
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
        void persistReplicationHarnessConfig();
        void updateReplicationDiagnostics();
        void writeClientReplicationStatus() const;

        [[nodiscard]] RECT getClientRect() const;

        IWindow* m_window = nullptr;
        Camera2D m_camera{};
        SimulationRuntime m_simulationRuntime{};
        RuntimeBoundaryReport m_runtimeBoundaryReport{};
        LocalDemoDiagnosticsReport m_localDemoDiagnosticsReport{};
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
        bool m_useHeadlessHostAuthority = false;
        std::string m_lastSnapshotReadError{};

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
