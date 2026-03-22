#pragma once

#include <string>

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
    class BgfxTextureAsset
    {
    public:
        ~BgfxTextureAsset();

        bool loadFromFile(const std::string& path, std::string& outStatus);
        void shutdown();

        [[nodiscard]] bool isReady() const;
        [[nodiscard]] const std::string& statusMessage() const;

#if WAR_HAS_BGFX
        [[nodiscard]] bgfx::TextureHandle handle() const;
#endif

    private:
        bool m_attemptedLoad = false;
        bool m_ready = false;
        std::string m_lastStatus = "bgfx texture not loaded";

#if WAR_HAS_BGFX
        bgfx::TextureHandle m_texture = BGFX_INVALID_HANDLE;
#endif
    };
}
