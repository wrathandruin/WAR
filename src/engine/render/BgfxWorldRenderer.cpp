#include "engine/render/BgfxWorldRenderer.h"

#include <cstring>
#include <vector>

#include "engine/render/BgfxViewTransform.h"

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

    bgfx::VertexLayout PosColorVertex::layout{};
    bool s_layoutInitialized = false;

    void ensureLayoutInitialized()
    {
        if (!s_layoutInitialized)
        {
            PosColorVertex::init();
            s_layoutInitialized = true;
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
        ensureLayoutInitialized();

        if (!m_colorProgram.loadColorProgram(m_statusMessage))
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

        submitLayer(renderData.tiles);
        submitLayer(renderData.path);
        submitLayer(renderData.hoveredTile);
        submitLayer(renderData.entities);
        submitLayer(renderData.player);

        m_statusMessage = "bgfx world rendered";
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
        m_colorProgram.shutdown();
        m_statusMessage = "bgfx world renderer shutdown";
    }

    const std::string& BgfxWorldRenderer::statusMessage() const
    {
        return m_statusMessage;
    }

    bool BgfxWorldRenderer::submitLayer(const BgfxRenderLayer& layer) const
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
}
