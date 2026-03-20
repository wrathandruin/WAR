#include "engine/render/RenderAssetPaths.h"

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

        return "assets/shaders/" + folder + "/vs_color.bin";
    }

    std::string colorFragmentShaderPath()
    {
        const std::string folder = shaderBackendFolder();
        if (folder.empty())
        {
            return "";
        }

        return "assets/shaders/" + folder + "/fs_color.bin";
    }
}
