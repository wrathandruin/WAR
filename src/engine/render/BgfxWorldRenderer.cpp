#include "engine/render/BgfxWorldRenderer.h"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

#if defined(__has_include)
#  if __has_include(<bgfx/bgfx.h>)
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
        namespace fs = std::filesystem;

        struct PosColorVertex
        {
            float x;
            float y;
            float z;
            uint32_t abgr;

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
        std::string s_statusMessage = "Program not attempted yet.";

        fs::path executableDirectory()
        {
            std::wstring buffer(MAX_PATH, L'\0');

            for (;;)
            {
                const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
                if (length == 0)
                {
                    return {};
                }

                if (length < buffer.size() - 1)
                {
                    buffer.resize(length);
                    return fs::path(buffer).parent_path();
                }

                buffer.resize(buffer.size() * 2);
            }
        }

        void appendIfUnique(std::vector<fs::path>& paths, const fs::path& candidate)
        {
            if (candidate.empty())
            {
                return;
            }

            for (const fs::path& existing : paths)
            {
                if (existing == candidate)
                {
                    return;
                }
            }

            paths.push_back(candidate);
        }

        std::vector<fs::path> shaderSearchRoots()
        {
            std::vector<fs::path> roots;

            std::error_code error;
            appendIfUnique(roots, fs::current_path(error));

            const fs::path exeDir = executableDirectory();
            appendIfUnique(roots, exeDir);
            appendIfUnique(roots, exeDir.parent_path());
            appendIfUnique(roots, exeDir.parent_path().parent_path());

            return roots;
        }

        fs::path resolveShaderPath(const std::string& folder, const char* fileName)
        {
            for (const fs::path& root : shaderSearchRoots())
            {
                const fs::path candidate = root / "assets" / "shaders" / folder / fileName;
                std::error_code error;
                if (fs::exists(candidate, error))
                {
                    return candidate;
                }
            }

            return {};
        }

        const char* rendererName(bgfx::RendererType::Enum type)
        {
            switch (type)
            {
            case bgfx::RendererType::Direct3D11:
                return "Direct3D11";
            case bgfx::RendererType::Direct3D12:
                return "Direct3D12";
            case bgfx::RendererType::OpenGL:
                return "OpenGL";
            case bgfx::RendererType::Vulkan:
                return "Vulkan";
            default:
                return "Unknown";
            }
        }

        std::string shaderFolderForRenderer(bgfx::RendererType::Enum type)
        {
            switch (type)
            {
            case bgfx::RendererType::Direct3D11:
                return "dx11";
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

        const bgfx::Memory* loadMemoryFromFile(const fs::path& path)
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

            const bgfx::Memory* memory = bgfx::alloc(static_cast<uint32_t>(size + 1));
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
                s_statusMessage = std::string("Unsupported renderer: ")
                    + rendererName(bgfx::getRendererType());
                return false;
            }

            const fs::path vsPath = resolveShaderPath(folder, "vs_color.bin");
            const fs::path fsPath = resolveShaderPath(folder, "fs_color.bin");
            if (vsPath.empty() || fsPath.empty())
            {
                s_statusMessage = "Shader files not found in assets/shaders/" + folder;
                return false;
            }

            const bgfx::Memory* vsMemory = loadMemoryFromFile(vsPath);
            const bgfx::Memory* fsMemory = loadMemoryFromFile(fsPath);
            if (vsMemory == nullptr || fsMemory == nullptr)
            {
                s_statusMessage = "Shader files were found but could not be read.";
                return false;
            }

            const bgfx::ShaderHandle vs = bgfx::createShader(vsMemory);
            const bgfx::ShaderHandle fs = bgfx::createShader(fsMemory);
            if (!bgfx::isValid(vs) || !bgfx::isValid(fs))
            {
                s_statusMessage = "bgfx rejected the compiled shader binaries.";
                return false;
            }

            s_program = bgfx::createProgram(vs, fs, true);
            s_programReady = bgfx::isValid(s_program);
            s_statusMessage = s_programReady
                ? std::string("Shader program ready from ") + folder + " binaries."
                : "bgfx failed to link the shader program.";
            return s_programReady;
        }

        uint32_t rgbaToAbgr(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        {
            return (static_cast<uint32_t>(a) << 24)
                 | (static_cast<uint32_t>(b) << 16)
                 | (static_cast<uint32_t>(g) << 8)
                 | static_cast<uint32_t>(r);
        }

        RECT tileToScreenRect(const WorldState& worldState, const Camera2D& camera, TileCoord tile)
        {
            const int tileSize = worldState.world().getTileSize();

            const Vec2 center = worldState.world().tileToWorldCenter(tile);
            const Vec2 topLeftWorld{
                center.x - static_cast<float>(tileSize) * 0.5f,
                center.y - static_cast<float>(tileSize) * 0.5f
            };
            const Vec2 bottomRightWorld{
                center.x + static_cast<float>(tileSize) * 0.5f,
                center.y + static_cast<float>(tileSize) * 0.5f
            };

            const Vec2 topLeft = camera.worldToScreen(topLeftWorld);
            const Vec2 bottomRight = camera.worldToScreen(bottomRightWorld);

            return RECT{
                static_cast<LONG>(topLeft.x),
                static_cast<LONG>(topLeft.y),
                static_cast<LONG>(bottomRight.x),
                static_cast<LONG>(bottomRight.y)
            };
        }

        RECT centeredScreenRect(const Camera2D& camera, const Vec2& worldPosition, float halfSize)
        {
            const Vec2 screen = camera.worldToScreen(worldPosition);

            return RECT{
                static_cast<LONG>(screen.x - halfSize),
                static_cast<LONG>(screen.y - halfSize),
                static_cast<LONG>(screen.x + halfSize),
                static_cast<LONG>(screen.y + halfSize)
            };
        }

        void appendQuad(
            std::vector<PosColorVertex>& vertices,
            std::vector<uint16_t>& indices,
            const RECT& rect,
            int width,
            int height,
            uint32_t color)
        {
            if (width <= 0 || height <= 0)
            {
                return;
            }

            const uint16_t base = static_cast<uint16_t>(vertices.size());

            const float left = (static_cast<float>(rect.left) / static_cast<float>(width)) * 2.0f - 1.0f;
            const float right = (static_cast<float>(rect.right) / static_cast<float>(width)) * 2.0f - 1.0f;
            const float top = 1.0f - (static_cast<float>(rect.top) / static_cast<float>(height)) * 2.0f;
            const float bottom = 1.0f - (static_cast<float>(rect.bottom) / static_cast<float>(height)) * 2.0f;

            vertices.push_back({ left,  top,    0.0f, color });
            vertices.push_back({ right, top,    0.0f, color });
            vertices.push_back({ right, bottom, 0.0f, color });
            vertices.push_back({ left,  bottom, 0.0f, color });

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        bool submitGeometry(
            const std::vector<PosColorVertex>& vertices,
            const std::vector<uint16_t>& indices,
            uint64_t stateFlags)
        {
            if (vertices.empty() || indices.empty())
            {
                return false;
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
            std::memcpy(tib.data, indices.data(), indices.size() * sizeof(uint16_t));

            bgfx::setVertexBuffer(0, &tvb);
            bgfx::setIndexBuffer(&tib);
            bgfx::setState(stateFlags);
            bgfx::submit(0, s_program);
            return true;
        }

        uint32_t tileColor(bool blocked)
        {
            return blocked
                ? rgbaToAbgr(220, 60, 60)
                : rgbaToAbgr(34, 38, 46);
        }

        uint32_t pathColor()
        {
            return rgbaToAbgr(255, 180, 90);
        }

        uint32_t playerColor()
        {
            return rgbaToAbgr(160, 210, 255);
        }

        uint32_t hoveredColor(bool blocked)
        {
            return blocked
                ? rgbaToAbgr(255, 140, 140, 140)
                : rgbaToAbgr(240, 230, 100, 140);
        }

        uint32_t entityColor(const Entity& entity)
        {
            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen
                    ? rgbaToAbgr(110, 110, 110)
                    : rgbaToAbgr(120, 255, 150);

            case EntityType::Terminal:
                return entity.isPowered
                    ? rgbaToAbgr(90, 170, 255)
                    : rgbaToAbgr(70, 110, 150);

            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return rgbaToAbgr(220, 110, 110);
                }
                return entity.isOpen
                    ? rgbaToAbgr(160, 160, 210)
                    : rgbaToAbgr(190, 190, 240);

            default:
                return rgbaToAbgr(120, 255, 150);
            }
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

        const bgfx::Caps* caps = bgfx::getCaps();
        if (caps == nullptr)
        {
            s_statusMessage = "bgfx returned no renderer caps.";
            return false;
        }

        const bgfx::Stats* stats = bgfx::getStats();
        const int viewWidth = stats ? static_cast<int>(stats->width) : 0;
        const int viewHeight = stats ? static_cast<int>(stats->height) : 0;

        if (viewWidth <= 0 || viewHeight <= 0)
        {
            s_statusMessage = "bgfx reported a zero-sized view.";
            return false;
        }

        const uint64_t opaqueState =
            BGFX_STATE_WRITE_RGB |
            BGFX_STATE_WRITE_A |
            BGFX_STATE_MSAA |
            BGFX_STATE_BLEND_ALPHA;

        std::vector<PosColorVertex> vertices;
        std::vector<uint16_t> indices;

        vertices.reserve(static_cast<size_t>(worldState.world().getWidth() * worldState.world().getHeight()) * 4);
        indices.reserve(static_cast<size_t>(worldState.world().getWidth() * worldState.world().getHeight()) * 6);

        for (int y = 0; y < worldState.world().getHeight(); ++y)
        {
            for (int x = 0; x < worldState.world().getWidth(); ++x)
            {
                const TileCoord tile{ x, y };
                appendQuad(
                    vertices,
                    indices,
                    tileToScreenRect(worldState, camera, tile),
                    viewWidth,
                    viewHeight,
                    tileColor(worldState.world().isBlocked(tile)));
            }
        }

        submitGeometry(vertices, indices, opaqueState);

        vertices.clear();
        indices.clear();

        for (size_t i = pathIndex; i < currentPath.size(); ++i)
        {
            const RECT rect = centeredScreenRect(
                camera,
                worldState.world().tileToWorldCenter(currentPath[i]),
                5.0f * camera.getZoom());

            appendQuad(vertices, indices, rect, viewWidth, viewHeight, pathColor());
        }

        submitGeometry(vertices, indices, opaqueState);

        vertices.clear();
        indices.clear();

        if (hasHoveredTile && worldState.world().isInBounds(hoveredTile))
        {
            appendQuad(
                vertices,
                indices,
                tileToScreenRect(worldState, camera, hoveredTile),
                viewWidth,
                viewHeight,
                hoveredColor(worldState.world().isBlocked(hoveredTile)));
        }

        submitGeometry(vertices, indices, opaqueState);

        vertices.clear();
        indices.clear();

        for (const Entity& entity : worldState.entities().all())
        {
            const RECT rect = centeredScreenRect(
                camera,
                worldState.world().tileToWorldCenter(entity.tile),
                8.0f * camera.getZoom());

            appendQuad(vertices, indices, rect, viewWidth, viewHeight, entityColor(entity));
        }

        submitGeometry(vertices, indices, opaqueState);

        vertices.clear();
        indices.clear();

        const RECT playerRect = centeredScreenRect(camera, playerPosition, 12.0f * camera.getZoom());
        appendQuad(vertices, indices, playerRect, viewWidth, viewHeight, playerColor());
        submitGeometry(vertices, indices, opaqueState);

        s_statusMessage = "Geometry path submitted successfully.";
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

    const char* BgfxWorldRenderer::statusMessage() const
    {
#if WAR_HAS_BGFX
        return s_statusMessage.c_str();
#else
        return "bgfx support is not compiled in.";
#endif
    }
}
