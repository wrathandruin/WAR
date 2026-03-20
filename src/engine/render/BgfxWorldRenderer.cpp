#include "engine/render/BgfxWorldRenderer.h"

#include <array>
#include <cstring>
#include <vector>

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
    bool BgfxWorldRenderer::render(
        const WorldState& worldState,
        const Camera2D& camera,
        const Vec2& playerPosition,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex,
        bool hasHoveredTile,
        TileCoord hoveredTile)
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

        if (!ensureTextureAssetsLoaded())
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
            currentPath,
            pathIndex,
            hasHoveredTile,
            hoveredTile);

        submitColorLayer(renderData.tiles);
        submitColorLayer(renderData.path);
        submitColorLayer(renderData.hoveredTile);
        submitTexturedLayer(renderData.entities);
        submitTexturedLayer(renderData.player);

        m_statusMessage = "bgfx world rendered with textured sprites";
        return true;
#else
        (void)worldState;
        (void)camera;
        (void)playerPosition;
        (void)currentPath;
        (void)pathIndex;
        (void)hasHoveredTile;
        (void)hoveredTile;
        m_statusMessage = "bgfx headers not available at compile time";
        return false;
#endif
    }

    void BgfxWorldRenderer::shutdown()
    {
        m_playerTexture.shutdown();
        m_crateTexture.shutdown();
        m_terminalTexture.shutdown();
        m_lockerTexture.shutdown();

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

    bool BgfxWorldRenderer::ensureTextureAssetsLoaded()
    {
        std::string status{};

        if (!m_playerTexture.loadFromBmpFile(RenderAssetPaths::textureAssetPath("player.bmp"), status))
        {
            m_statusMessage = status;
            return false;
        }

        if (!m_crateTexture.loadFromBmpFile(RenderAssetPaths::textureAssetPath("crate.bmp"), status))
        {
            m_statusMessage = status;
            return false;
        }

        if (!m_terminalTexture.loadFromBmpFile(RenderAssetPaths::textureAssetPath("terminal.bmp"), status))
        {
            m_statusMessage = status;
            return false;
        }

        if (!m_lockerTexture.loadFromBmpFile(RenderAssetPaths::textureAssetPath("locker.bmp"), status))
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

            vertices.push_back({ quad.left,  quad.top,    0.0f, quad.color });
            vertices.push_back({ quad.right, quad.top,    0.0f, quad.color });
            vertices.push_back({ quad.right, quad.bottom, 0.0f, quad.color });
            vertices.push_back({ quad.left,  quad.bottom, 0.0f, quad.color });

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        bgfx::TransientVertexBuffer tvb{};
        bgfx::TransientIndexBuffer tib{};

        if (!bgfx::allocTransientBuffers(
            &tvb,
            PosColorVertex::layout,
            static_cast<uint32_t>(vertices.size()),
            &tib,
            static_cast<uint32_t>(indices.size())))
        {
            return false;
        }

        std::memcpy(tvb.data, vertices.data(), vertices.size() * sizeof(PosColorVertex));
        std::memcpy(tib.data, indices.data(), indices.size() * sizeof(unsigned short));

        bgfx::setVertexBuffer(0, &tvb);
        bgfx::setIndexBuffer(&tib);
        bgfx::setState(
            BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA);
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
        if (layer.quads.empty() || !m_textureProgram.isReady())
        {
            return false;
        }

        bool submittedAny = false;

        for (const BgfxTexturedQuad& quad : layer.quads)
        {
            const bgfx::TextureHandle texture = textureHandleFor(quad.texture);
            if (!bgfx::isValid(texture))
            {
                continue;
            }

            const std::array<PosTexColorVertex, 4> vertices{{
                { quad.left,  quad.top,    0.0f, quad.u0, quad.v0, quad.color },
                { quad.right, quad.top,    0.0f, quad.u1, quad.v0, quad.color },
                { quad.right, quad.bottom, 0.0f, quad.u1, quad.v1, quad.color },
                { quad.left,  quad.bottom, 0.0f, quad.u0, quad.v1, quad.color }
            }};

            constexpr std::array<unsigned short, 6> indices{{
                0, 1, 2,
                0, 2, 3
            }};

            bgfx::TransientVertexBuffer tvb{};
            bgfx::TransientIndexBuffer tib{};

            if (!bgfx::allocTransientBuffers(
                &tvb,
                PosTexColorVertex::layout,
                static_cast<uint32_t>(vertices.size()),
                &tib,
                static_cast<uint32_t>(indices.size())))
            {
                continue;
            }

            std::memcpy(tvb.data, vertices.data(), vertices.size() * sizeof(PosTexColorVertex));
            std::memcpy(tib.data, indices.data(), indices.size() * sizeof(unsigned short));

            bgfx::setVertexBuffer(0, &tvb);
            bgfx::setIndexBuffer(&tib);
            bgfx::setTexture(0, s_textureSampler, texture);
            bgfx::setState(
                BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_MSAA
                | BGFX_STATE_BLEND_ALPHA);
            bgfx::submit(0, m_textureProgram.handle());
            submittedAny = true;
        }

        return submittedAny;
#else
        (void)layer;
        return false;
#endif
    }

#if WAR_HAS_BGFX
    bgfx::TextureHandle BgfxWorldRenderer::textureHandleFor(BgfxTextureAssetId texture) const
    {
        switch (texture)
        {
        case BgfxTextureAssetId::Player:
            return m_playerTexture.handle();

        case BgfxTextureAssetId::Crate:
            return m_crateTexture.handle();

        case BgfxTextureAssetId::Terminal:
            return m_terminalTexture.handle();

        case BgfxTextureAssetId::Locker:
            return m_lockerTexture.handle();

        default:
            return BGFX_INVALID_HANDLE;
        }
    }
#endif
}
