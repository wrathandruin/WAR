#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "engine/core/RuntimePaths.h"
#include "engine/math/Vec2.h"
#include "engine/render/BgfxRenderData.h"
#include "engine/render/BgfxShaderProgram.h"
#include "engine/render/BgfxTextureAsset.h"
#include "engine/render/Camera2D.h"
#include "engine/world/WorldGrid.h"
#include "engine/world/WorldState.h"

namespace war
{
    class BgfxWorldRenderer
    {
    public:
        [[nodiscard]] bool render(
            const WorldState& worldState,
            const Camera2D& camera,
            const Vec2& playerPosition,
            const std::vector<TileCoord>& currentPath,
            size_t pathIndex,
            bool hasHoveredTile,
            TileCoord hoveredTile,
            bool hasSelectedTile,
            TileCoord selectedTile,
            bool hasActionTargetTile,
            TileCoord actionTargetTile,
            const RuntimeBoundaryReport& runtimeBoundaryReport);

        void shutdown();

        [[nodiscard]] const std::string& statusMessage() const;

    private:
        [[nodiscard]] bool ensureAtlasTextureLoaded();
        [[nodiscard]] bool submitColorLayer(const BgfxRenderLayer& layer) const;
        [[nodiscard]] bool submitTexturedLayer(const BgfxTexturedRenderLayer& layer) const;

#if WAR_HAS_BGFX
        [[nodiscard]] bgfx::TextureHandle atlasTextureHandle() const;
#endif

        BgfxShaderProgram m_colorProgram{};
        BgfxShaderProgram m_textureProgram{};
        BgfxTextureAsset m_spriteAtlasTexture{};

        std::string m_statusMessage = "BgfxWorldRenderer not used yet";
    };
}
