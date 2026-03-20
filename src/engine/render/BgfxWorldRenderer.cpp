#include "engine/render/BgfxWorldRenderer.h"

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

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

        bool ensureProgramLoaded()
        {
            if (s_programAttempted)
            {
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
                return false;
            }

            const std::string vsPath = "assets/shaders/" + folder + "/vs_color.bin";
            const std::string fsPath = "assets/shaders/" + folder + "/fs_color.bin";

            const bgfx::Memory* vsMemory = loadMemoryFromFile(vsPath.c_str());
            const bgfx::Memory* fsMemory = loadMemoryFromFile(fsPath.c_str());
            if (vsMemory == nullptr || fsMemory == nullptr)
            {
                return false;
            }

            const bgfx::ShaderHandle vs = bgfx::createShader(vsMemory);
            const bgfx::ShaderHandle fs = bgfx::createShader(fsMemory);
            if (!bgfx::isValid(vs) || !bgfx::isValid(fs))
            {
                return false;
            }

            s_program = bgfx::createProgram(vs, fs, true);
            s_programReady = bgfx::isValid(s_program);
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
        TileCoord hoveredTile) const
    {
#if WAR_HAS_BGFX
        if (!ensureProgramLoaded())
        {
            return false;
        }

        const bgfx::Stats* stats = bgfx::getStats();
        if (stats == nullptr)
        {
            return false;
        }

        const int viewWidth = static_cast<int>(stats->width);
        const int viewHeight = static_cast<int>(stats->height);

        if (viewWidth <= 0 || viewHeight <= 0)
        {
            return false;
        }

        const BgfxWorldRenderData renderData = BgfxRenderDataBuilder::build(
            worldState,
            camera,
            playerPosition,
            currentPath,
            pathIndex,
            hasHoveredTile,
            hoveredTile);

        submitLayer(renderData.tiles, viewWidth, viewHeight);
        submitLayer(renderData.path, viewWidth, viewHeight);
        submitLayer(renderData.hoveredTile, viewWidth, viewHeight);
        submitLayer(renderData.entities, viewWidth, viewHeight);
        submitLayer(renderData.player, viewWidth, viewHeight);

        return true;
#else
        (void)worldState;
        (void)camera;
        (void)playerPosition;
        (void)currentPath;
        (void)pathIndex;
        (void)hasHoveredTile;
        (void)hoveredTile;
        return false;
#endif
    }

    bool BgfxWorldRenderer::submitLayer(const BgfxRenderLayer& layer, int viewWidth, int viewHeight) const
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

            const float left = (static_cast<float>(quad.rect.left) / static_cast<float>(viewWidth)) * 2.0f - 1.0f;
            const float right = (static_cast<float>(quad.rect.right) / static_cast<float>(viewWidth)) * 2.0f - 1.0f;
            const float top = 1.0f - (static_cast<float>(quad.rect.top) / static_cast<float>(viewHeight)) * 2.0f;
            const float bottom = 1.0f - (static_cast<float>(quad.rect.bottom) / static_cast<float>(viewHeight)) * 2.0f;

            vertices.push_back({ left,  top,    0.0f, quad.color });
            vertices.push_back({ right, top,    0.0f, quad.color });
            vertices.push_back({ right, bottom, 0.0f, quad.color });
            vertices.push_back({ left,  bottom, 0.0f, quad.color });

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        bgfx::TransientVertexBuffer tvb{};
        bgfx::TransientIndexBuffer tib{};

        if (!bgfx::allocTransientVertexBuffer(&tvb, static_cast<unsigned int>(vertices.size()), PosColorVertex::layout))
        {
            return false;
        }

        if (!bgfx::allocTransientIndexBuffer(&tib, static_cast<unsigned int>(indices.size())))
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
        (void)viewWidth;
        (void)viewHeight;
        return false;
#endif
    }
}
