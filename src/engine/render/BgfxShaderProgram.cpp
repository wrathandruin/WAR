#include "engine/render/BgfxShaderProgram.h"

#include <fstream>

#include "engine/render/RenderAssetPaths.h"

namespace war
{
#if WAR_HAS_BGFX
    namespace
    {
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

            const bgfx::Memory* memory = bgfx::alloc(static_cast<uint32_t>(size + 1));
            if (!file.read(reinterpret_cast<char*>(memory->data), size))
            {
                return nullptr;
            }

            memory->data[size] = 0;
            return memory;
        }
    }
#endif

    BgfxShaderProgram::~BgfxShaderProgram()
    {
        shutdown();
    }

    bool BgfxShaderProgram::loadColorProgram(std::string& outStatus)
    {
#if WAR_HAS_BGFX
        if (m_attemptedLoad)
        {
            outStatus = m_lastStatus;
            return m_ready;
        }

        m_attemptedLoad = true;

        const std::string vsPath = RenderAssetPaths::colorVertexShaderPath();
        const std::string fsPath = RenderAssetPaths::colorFragmentShaderPath();

        if (vsPath.empty() || fsPath.empty())
        {
            m_lastStatus = "unsupported bgfx renderer for shader asset folder";
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::Memory* vsMemory = loadMemoryFromFile(vsPath.c_str());
        if (vsMemory == nullptr)
        {
            m_lastStatus = "missing shader binary: " + vsPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::Memory* fsMemory = loadMemoryFromFile(fsPath.c_str());
        if (fsMemory == nullptr)
        {
            m_lastStatus = "missing shader binary: " + fsPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::ShaderHandle vs = bgfx::createShader(vsMemory);
        if (!bgfx::isValid(vs))
        {
            m_lastStatus = "failed to create bgfx vertex shader: " + vsPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::ShaderHandle fs = bgfx::createShader(fsMemory);
        if (!bgfx::isValid(fs))
        {
            bgfx::destroy(vs);
            m_lastStatus = "failed to create bgfx fragment shader: " + fsPath;
            outStatus = m_lastStatus;
            return false;
        }

        m_program = bgfx::createProgram(vs, fs, true);
        if (!bgfx::isValid(m_program))
        {
            m_lastStatus = "failed to create bgfx shader program";
            outStatus = m_lastStatus;
            return false;
        }

        m_ready = true;
        m_lastStatus = "bgfx color program ready";
        outStatus = m_lastStatus;
        return true;
#else
        m_lastStatus = "bgfx headers not available at compile time";
        outStatus = m_lastStatus;
        return false;
#endif
    }

    void BgfxShaderProgram::shutdown()
    {
#if WAR_HAS_BGFX
        if (bgfx::isValid(m_program))
        {
            bgfx::destroy(m_program);
            m_program = BGFX_INVALID_HANDLE;
        }
#endif

        m_ready = false;
        m_attemptedLoad = false;
        m_lastStatus = "bgfx color program not loaded";
    }

    bool BgfxShaderProgram::isReady() const
    {
        return m_ready;
    }

    const std::string& BgfxShaderProgram::statusMessage() const
    {
        return m_lastStatus;
    }

#if WAR_HAS_BGFX
    bgfx::ProgramHandle BgfxShaderProgram::handle() const
    {
        return m_program;
    }
#endif
}
