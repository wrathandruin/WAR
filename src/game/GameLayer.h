#pragma once

#include <memory>
#include <string>
#include <vector>

#include <windows.h>

#include "engine/gameplay/ActionQueue.h"
#include "engine/math/Vec2.h"
#include "engine/render/BgfxDebugFrameRenderer.h"
#include "engine/render/BgfxWorldRenderer.h"
#include "engine/render/BgfxWorldTheme.h"
#include "engine/render/Camera2D.h"
#include "engine/render/DebugOverlayRenderer.h"
#include "engine/render/IRenderDevice.h"
#include "engine/render/WorldRenderer.h"
#include "engine/world/WorldGrid.h"
#include "engine/world/WorldState.h"
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
        void updatePlayer(float dt);
        void pushEvent(const std::string& message);
        void applyAuthoringHotkeys();

        [[nodiscard]] RECT getClientRect() const;

        IWindow* m_window = nullptr;
        Camera2D m_camera{};
        WorldState m_worldState{};
        ActionQueue m_actions{};
        WorldRenderer m_worldRenderer{};
        DebugOverlayRenderer m_debugOverlayRenderer{};
        BgfxWorldRenderer m_bgfxWorldRenderer{};
        BgfxDebugFrameRenderer m_bgfxDebugFrameRenderer{};
        std::unique_ptr<IRenderDevice> m_renderDevice{};

        Vec2 m_playerPosition{ 0.0f, 0.0f };
        float m_playerSpeed = 210.0f;

        TileCoord m_hoveredTile{};
        bool m_hasHoveredTile = false;

        TileCoord m_selectedTile{};
        bool m_hasSelectedTile = false;

        TileCoord m_actionTargetTile{};
        bool m_hasActionTargetTile = false;

        std::vector<TileCoord> m_currentPath;
        size_t m_pathIndex = 0;

        float m_lastDeltaTime = 0.016f;
        std::vector<std::string> m_eventLog;

        bool m_overlayKeyWasDown = false;
        bool m_hotspotKeyWasDown = false;
        bool m_palette7WasDown = false;
        bool m_palette8WasDown = false;
        bool m_palette9WasDown = false;
    };
}
