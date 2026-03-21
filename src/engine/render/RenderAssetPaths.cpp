#include "engine/render/RenderAssetPaths.h"

#include <filesystem>

#include "engine/core/RuntimePaths.h"

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

namespace war::RenderAssetPaths
{
    namespace
    {
        std::string assetPathString(const std::filesystem::path& relativePath)
        {
            const RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            if (runtimeBoundaryReport.assetRootResolved)
            {
                return (runtimeBoundaryReport.assetRoot / relativePath).generic_string();
            }

            return (std::filesystem::path("assets") / relativePath).generic_string();
        }
    }

    std::string shaderBackendFolder()
    {
#if WAR_HAS_BGFX
        switch (bgfx::getRendererType())
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
#else
        return "";
#endif
    }

    std::string colorVertexShaderPath()
    {
        const std::string folder = shaderBackendFolder();
        if (folder.empty())
        {
            return "";
        }

        return assetPathString(std::filesystem::path("shaders") / folder / "vs_color.bin");
    }

    std::string colorFragmentShaderPath()
    {
        const std::string folder = shaderBackendFolder();
        if (folder.empty())
        {
            return "";
        }

        return assetPathString(std::filesystem::path("shaders") / folder / "fs_color.bin");
    }

    std::string textureVertexShaderPath()
    {
        const std::string folder = shaderBackendFolder();
        if (folder.empty())
        {
            return "";
        }

        return assetPathString(std::filesystem::path("shaders") / folder / "vs_texture.bin");
    }

    std::string textureFragmentShaderPath()
    {
        const std::string folder = shaderBackendFolder();
        if (folder.empty())
        {
            return "";
        }

        return assetPathString(std::filesystem::path("shaders") / folder / "fs_texture.bin");
    }

    std::string textureAssetPath(const std::string& fileName)
    {
        return assetPathString(std::filesystem::path("textures") / fileName);
    }

    std::string spriteAtlasTexturePath()
    {
        return textureAssetPath("world_atlas.bmp");
    }
}
