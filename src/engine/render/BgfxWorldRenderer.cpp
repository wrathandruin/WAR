#include "engine/render/BgfxWorldRenderer.h"

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "engine/render/BgfxViewTransform.h"

#if defined(__has_include)
#  if __has_include(<bgfx/bgfx.h>) && __has_include(<bx/math.h>)
#    define WAR_HAS_BGFX 1
#  else
#    define WAR_HAS_BGFX 0
#  endif
#else
#  define WAR_HAS_BGFX 0
#endif

#if WAR_HAS_BGFX
#  include <bgfx/bgfx.h>
#endif

namespace war
{
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
        bool s_programAttempted = false;
        bool s_programReady = false;
        bgfx::ProgramHandle s_program = BGFX_INVALID_HANDLE;

        std::string shaderFolderForRenderer(bgfx::RendererType::Enum type)
        {
            switch (type)
            {
            case bgfx::RendererType::Direct3D11:
            case bgfx::RendererType::Direct3D12:
                return "dx11";

            case bgfx::RendererType::OpenGL:
                return "glsl";

            case bgfx::RendererType::Vulkan:
                return "spirv";

            default:
                return "";
            }
        }

        const bgfx::Memory* loadMemoryFromFile(const char* path)
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file)
            {
                return nullptr;
            }

            const std::streamsize size = file.tellg();
            if (size <= 0)
            {
                return nullptr;
            }

            file.seekg(0, std::ios::beg);

            const bgfx::Memory* memory = bgfx::alloc(static_cast<unsigned int>(size + 1));
            if (!file.read(reinterpret_cast<char*>(memory->data), size))
            {
                return nullptr;
            }

            memory->data[size] = 0;
            return memory;
        }

        bool ensureProgramLoaded(std::string& statusMessage)
        {
            if (s_programAttempted)
            {
                statusMessage = s_programReady
                    ? "bgfx color program ready"
                    : "bgfx shaders missing or failed to load";
                return s_programReady;
            }

            s_programAttempted = true;

            if (!s_layoutInitialized)
            {
                PosColorVertex::init();
                s_layoutInitialized = true;
            }

            const std::string folder = shaderFolderForRenderer(bgfx::getRendererType());
            if (folder.empty())
            {
                statusMessage = "unsupported bgfx renderer for shader folder mapping";
                return false;
            }

            const std::string vsPath = "assets/shaders/" + folder + "/vs_color.bin";
            const std::string fsPath = "assets/shaders/" + folder + "/fs_color.bin";

            const bgfx::Memory* vsMemory = loadMemoryFromFile(vsPath.c_str());
            const bgfx::Memory* fsMemory = loadMemoryFromFile(fsPath.c_str());
            if (vsMemory == nullptr || fsMemory == nullptr)
            {
                statusMessage = "missing shader binaries: " + vsPath + " or " + fsPath;
                return false;
            }

            const bgfx::ShaderHandle vs = bgfx::createShader(vsMemory);
            const bgfx::ShaderHandle fs = bgfx::createShader(fsMemory);
            if (!bgfx::isValid(vs) || !bgfx::isValid(fs))
            {
                statusMessage = "failed to create bgfx shader handles";
                return false;
            }

            s_program = bgfx::createProgram(vs, fs, true);
            s_programReady = bgfx::isValid(s_program);
            statusMessage = s_programReady
                ? "bgfx color program ready"
                : "failed to create bgfx shader program";
            return s_programReady;
        }
    }
#endif

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
        if (!ensureProgramLoaded(m_statusMessage))
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

        bgfx::setViewRect(0, 0, 0, static_cast<unsigned short>(viewWidth), static_cast<unsigned short>(viewHeight));
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

    const std::string& BgfxWorldRenderer::statusMessage() const
    {
        return m_statusMessage;
    }

    bool BgfxWorldRenderer::submitLayer(const BgfxRenderLayer& layer) const
    {
#if WAR_HAS_BGFX
        if (layer.quads.empty())
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
        bgfx::submit(0, s_program);
        return true;
#else
        (void)layer;
        return false;
#endif
    }
}
