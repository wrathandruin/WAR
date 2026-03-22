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
    class BgfxShaderProgram
    {
    public:
        ~BgfxShaderProgram();

        bool loadColorProgram(std::string& outStatus);
        bool loadTextureProgram(std::string& outStatus);
        void shutdown();

        [[nodiscard]] bool isReady() const;
        [[nodiscard]] const std::string& statusMessage() const;

#if WAR_HAS_BGFX
        [[nodiscard]] bgfx::ProgramHandle handle() const;
#endif

    private:
        bool loadProgram(
            const std::string& vertexShaderPath,
            const std::string& fragmentShaderPath,
            const char* readyMessage,
            std::string& outStatus);

        bool m_attemptedLoad = false;
        bool m_ready = false;
        std::string m_lastStatus = "bgfx shader program not loaded";

#if WAR_HAS_BGFX
        bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
#endif
    };
}
