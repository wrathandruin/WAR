#include "engine/render/BgfxWorldRenderer.h"

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "engine/gameplay/Entity.h"
#include "engine/render/BgfxSpriteMaterial.h"
#include "engine/render/BgfxViewTransform.h"
#include "engine/render/RenderAssetPaths.h"

#if WAR_HAS_BGFX
namespace
{
    struct PosColorVertex
    {
        float x;
        float y;
        float z;
        unsigned int abgr;

        static bgfx::VertexLayout layout;

        static void init()
        {
            layout.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
        }
    };

    struct PosTexColorVertex
    {
        float x;
        float y;
        float z;
        float u;
        float v;
        unsigned int abgr;

        static bgfx::VertexLayout layout;

        static void init()
        {
            layout.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
        }
    };

    bgfx::VertexLayout PosColorVertex::layout{};
    bgfx::VertexLayout PosTexColorVertex::layout{};

    bool s_layoutsInitialized = false;
    bgfx::UniformHandle s_textureSampler = BGFX_INVALID_HANDLE;

    void ensureSharedBgfxState()
    {
        if (!s_layoutsInitialized)
        {
            PosColorVertex::init();
            PosTexColorVertex::init();
            s_layoutsInitialized = true;
        }

        if (!bgfx::isValid(s_textureSampler))
        {
            s_textureSampler = bgfx::createUniform("s_texColor0", bgfx::UniformType::Sampler);
        }
    }

    void shutdownSharedBgfxState()
    {
        if (bgfx::isValid(s_textureSampler))
        {
            bgfx::destroy(s_textureSampler);
            s_textureSampler = BGFX_INVALID_HANDLE;
        }
    }
}
#endif

namespace war
{
    namespace
    {
        std::string tileText(bool hasTile, TileCoord tile)
        {
            if (!hasTile)
            {
                return "none";
            }

            return "(" + std::to_string(tile.x) + ", " + std::to_string(tile.y) + ")";
        }

        std::string contextPrompt(const WorldState& worldState, bool hasHoveredTile, TileCoord hoveredTile)
        {
            if (!hasHoveredTile || !worldState.world().isInBounds(hoveredTile))
            {
                return "hover a tile to preview actions";
            }

            if (worldState.world().isBlocked(hoveredTile))
            {
                return "blocked tile | Shift+RMB inspect";
            }

            const Entity* entity = worldState.entities().getAt(hoveredTile);
            if (entity != nullptr)
            {
                return std::string("RMB interact ") + entity->name;
            }

            const WorldAuthoringHotspot* hotspot = worldState.authoringHotspotAt(hoveredTile);
            if (hotspot != nullptr)
            {
                return std::string("RMB use ") + hotspot->label;
            }

            return "LMB move | Shift+RMB inspect terrain";
        }

        std::string buildStatusMessage(
            const WorldState& worldState,
            const std::vector<TileCoord>& currentPath,
            size_t pathIndex,
            bool hasHoveredTile,
            TileCoord hoveredTile,
            bool hasSelectedTile,
            TileCoord selectedTile,
            bool hasActionTargetTile,
            TileCoord actionTargetTile,
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const LocalDemoDiagnosticsReport& localDemoDiagnosticsReport,
            const SharedSimulationDiagnostics& simulationDiagnostics,
            const HeadlessHostPresenceReport& headlessHostPresenceReport,
            const AuthoritativeHostProtocolReport& authoritativeHostProtocolReport)
        {
            const std::string hovered = tileText(hasHoveredTile && worldState.world().isInBounds(hoveredTile), hoveredTile);
            const std::string selected = tileText(hasSelectedTile && worldState.world().isInBounds(selectedTile), selectedTile);
            const std::string moveTarget = tileText(hasActionTargetTile && worldState.world().isInBounds(actionTargetTile), actionTargetTile);
            const std::string pathDestination =
                currentPath.empty() || pathIndex >= currentPath.size()
                    ? "none"
                    : tileText(true, currentPath.back());

            std::ostringstream status;
            status
                << "M40 combat lane | hp/armor/suit: "
                << simulationDiagnostics.playerHealth << "/"
                << simulationDiagnostics.playerArmor << "/"
                << simulationDiagnostics.suitIntegrity
                << " | combat: " << (simulationDiagnostics.combatActive ? "active" : "idle")
                << " | round: " << simulationDiagnostics.combatRoundNumber
                << " | hostile: " << simulationDiagnostics.hostileLabel
                << " " << simulationDiagnostics.hostileHealth << "/"
                << simulationDiagnostics.hostileMaxHealth
                << " | hazard: " << simulationDiagnostics.currentHazardLabel
                << " | oxygen: " << simulationDiagnostics.oxygenSecondsRemaining
                << " | inv: " << simulationDiagnostics.inventoryItemCount
                << " | wins: " << simulationDiagnostics.encounterWins
                << " | runtime: " << (runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged")
                << " | packaged lane: " << (localDemoDiagnosticsReport.packagedLaneReady ? "ready" : "not staged")
                << " | host online: " << (headlessHostPresenceReport.hostOnline ? "yes" : "no")
                << " | protocol lane: " << (authoritativeHostProtocolReport.authorityLaneReady ? "ready" : "not ready")
                << " | hover: " << hovered
                << " | prompt: " << contextPrompt(worldState, hasHoveredTile, hoveredTile)
                << " | selected: " << selected
                << " | move target: " << moveTarget
                << " | path destination: " << pathDestination;
            return status.str();
        }
    }

    bool BgfxWorldRenderer::render(
        const WorldState& worldState,
        const Camera2D& camera,
        const Vec2& playerPosition,
        const std::vector<RemotePresenceRenderProxy>& remotePresenceProxies,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex,
        bool hasHoveredTile,
        TileCoord hoveredTile,
        bool hasSelectedTile,
        TileCoord selectedTile,
        bool hasActionTargetTile,
        TileCoord actionTargetTile,
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const LocalDemoDiagnosticsReport& localDemoDiagnosticsReport,
        const SharedSimulationDiagnostics& simulationDiagnostics,
        const HeadlessHostPresenceReport& headlessHostPresenceReport,
        const AuthoritativeHostProtocolReport& authoritativeHostProtocolReport)
    {
#if WAR_HAS_BGFX
        ensureSharedBgfxState();

        if (!m_colorProgram.loadColorProgram(m_statusMessage))
        {
            return false;
        }

        if (!m_textureProgram.loadTextureProgram(m_statusMessage))
        {
            return false;
        }

        if (!ensureAtlasTextureLoaded())
        {
            return false;
        }

        const bgfx::Stats* stats = bgfx::getStats();
        if (stats == nullptr)
        {
            m_statusMessage = "bgfx stats unavailable";
            return false;
        }

        const int viewWidth = static_cast<int>(stats->width);
        const int viewHeight = static_cast<int>(stats->height);

        if (viewWidth <= 0 || viewHeight <= 0)
        {
            m_statusMessage = "invalid bgfx view size";
            return false;
        }

        float view[16]{};
        float proj[16]{};
        BgfxViewTransform::buildMatrices(camera, viewWidth, viewHeight, view, proj);

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(viewWidth), static_cast<uint16_t>(viewHeight));
        bgfx::setViewTransform(0, view, proj);

        const BgfxWorldRenderData renderData = BgfxRenderDataBuilder::build(
            worldState,
            camera,
            playerPosition,
            remotePresenceProxies,
            currentPath,
            pathIndex,
            hasHoveredTile,
            hoveredTile,
            hasSelectedTile,
            selectedTile,
            hasActionTargetTile,
            actionTargetTile);

        const bool tilesSubmitted = submitTexturedLayer(renderData.tiles);
        const bool actorsSubmitted = submitTexturedLayer(renderData.actors);
        (void)submitColorLayer(renderData.regionOverlay);
        (void)submitColorLayer(renderData.path);
        (void)submitColorLayer(renderData.selectedTile);
        (void)submitColorLayer(renderData.actionTarget);
        (void)submitColorLayer(renderData.authoringHotspots);
        (void)submitColorLayer(renderData.hoveredTile);

        if (!tilesSubmitted || !actorsSubmitted)
        {
            m_statusMessage = "bgfx critical layer submission failed";
            return false;
        }

        m_statusMessage = buildStatusMessage(
            worldState,
            currentPath,
            pathIndex,
            hasHoveredTile,
            hoveredTile,
            hasSelectedTile,
            selectedTile,
            hasActionTargetTile,
            actionTargetTile,
            runtimeBoundaryReport,
            localDemoDiagnosticsReport,
            simulationDiagnostics,
            headlessHostPresenceReport,
            authoritativeHostProtocolReport);
        return true;
#else
        (void)worldState;
        (void)camera;
        (void)playerPosition;
        (void)currentPath;
        (void)pathIndex;
        (void)hasHoveredTile;
        (void)hoveredTile;
        (void)hasSelectedTile;
        (void)selectedTile;
        (void)hasActionTargetTile;
        (void)actionTargetTile;
        (void)runtimeBoundaryReport;
        (void)localDemoDiagnosticsReport;
        (void)simulationDiagnostics;
        (void)headlessHostPresenceReport;
        (void)authoritativeHostProtocolReport;
        m_statusMessage = "bgfx headers not available at compile time";
        return false;
#endif
    }

    void BgfxWorldRenderer::shutdown()
    {
        m_spriteAtlasTexture.shutdown();
        m_textureProgram.shutdown();
        m_colorProgram.shutdown();

#if WAR_HAS_BGFX
        shutdownSharedBgfxState();
#endif
        m_statusMessage = "bgfx world renderer shutdown";
    }

    const std::string& BgfxWorldRenderer::statusMessage() const
    {
        return m_statusMessage;
    }

    bool BgfxWorldRenderer::ensureAtlasTextureLoaded()
    {
        std::string status{};
        if (!m_spriteAtlasTexture.loadFromFile(RenderAssetPaths::spriteAtlasTexturePath(), status))
        {
            m_statusMessage = status;
            return false;
        }

        return true;
    }

    bool BgfxWorldRenderer::submitColorLayer(const BgfxRenderLayer& layer) const
    {
#if WAR_HAS_BGFX
        if (layer.quads.empty() || !m_colorProgram.isReady())
        {
            return false;
        }

        std::vector<PosColorVertex> vertices;
        std::vector<unsigned short> indices;
        vertices.reserve(layer.quads.size() * 4);
        indices.reserve(layer.quads.size() * 6);

        for (const BgfxQuad& quad : layer.quads)
        {
            const unsigned short base = static_cast<unsigned short>(vertices.size());

            vertices.push_back({ quad.left, quad.top, 0.0f, quad.color });
            vertices.push_back({ quad.right, quad.top, 0.0f, quad.color });
            vertices.push_back({ quad.right, quad.bottom, 0.0f, quad.color });
            vertices.push_back({ quad.left, quad.bottom, 0.0f, quad.color });

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        bgfx::TransientVertexBuffer tvb{};
        bgfx::TransientIndexBuffer tib{};

        if (!bgfx::allocTransientBuffers(&tvb, PosColorVertex::layout, static_cast<uint32_t>(vertices.size()), &tib, static_cast<uint32_t>(indices.size())))
        {
            return false;
        }

        std::memcpy(tvb.data, vertices.data(), vertices.size() * sizeof(PosColorVertex));
        std::memcpy(tib.data, indices.data(), indices.size() * sizeof(unsigned short));

        bgfx::setVertexBuffer(0, &tvb);
        bgfx::setIndexBuffer(&tib);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA);
        bgfx::submit(0, m_colorProgram.handle());
        return true;
#else
        (void)layer;
        return false;
#endif
    }

    bool BgfxWorldRenderer::submitTexturedLayer(const BgfxTexturedRenderLayer& layer) const
    {
#if WAR_HAS_BGFX
        if (layer.quads.empty() || !m_textureProgram.isReady() || !m_spriteAtlasTexture.isReady())
        {
            return false;
        }

        std::vector<PosTexColorVertex> vertices;
        std::vector<unsigned short> indices;
        vertices.reserve(layer.quads.size() * 4);
        indices.reserve(layer.quads.size() * 6);

        for (const BgfxTexturedQuad& quad : layer.quads)
        {
            const BgfxUvRect uv = BgfxSpriteMaterials::uvFor(quad.material);
            const unsigned short base = static_cast<unsigned short>(vertices.size());

            vertices.push_back({ quad.left, quad.top, 0.0f, uv.u0, uv.v0, quad.color });
            vertices.push_back({ quad.right, quad.top, 0.0f, uv.u1, uv.v0, quad.color });
            vertices.push_back({ quad.right, quad.bottom, 0.0f, uv.u1, uv.v1, quad.color });
            vertices.push_back({ quad.left, quad.bottom, 0.0f, uv.u0, uv.v1, quad.color });

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        bgfx::TransientVertexBuffer tvb{};
        bgfx::TransientIndexBuffer tib{};

        if (!bgfx::allocTransientBuffers(&tvb, PosTexColorVertex::layout, static_cast<uint32_t>(vertices.size()), &tib, static_cast<uint32_t>(indices.size())))
        {
            return false;
        }

        std::memcpy(tvb.data, vertices.data(), vertices.size() * sizeof(PosTexColorVertex));
        std::memcpy(tib.data, indices.data(), indices.size() * sizeof(unsigned short));

        bgfx::setVertexBuffer(0, &tvb);
        bgfx::setIndexBuffer(&tib);
        bgfx::setTexture(0, s_textureSampler, atlasTextureHandle());
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA);
        bgfx::submit(0, m_textureProgram.handle());
        return true;
#else
        (void)layer;
        return false;
#endif
    }

#if WAR_HAS_BGFX
    bgfx::TextureHandle BgfxWorldRenderer::atlasTextureHandle() const
    {
        return m_spriteAtlasTexture.handle();
    }
#endif
}
