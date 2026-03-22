#include "engine/render/BgfxShaderProgram.h"

#include <cstdint>
#include <fstream>
#include <vector>

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

            std::vector<char> bytes(static_cast<size_t>(size) + 1u, '\0');
            if (!file.read(bytes.data(), size))
            {
                return nullptr;
            }

            return bgfx::copy(bytes.data(), static_cast<uint32_t>(bytes.size()));
        }
    }
#endif

    BgfxShaderProgram::~BgfxShaderProgram()
    {
        shutdown();
    }

    bool BgfxShaderProgram::loadColorProgram(std::string& outStatus)
    {
        return loadProgram(
            RenderAssetPaths::colorVertexShaderPath(),
            RenderAssetPaths::colorFragmentShaderPath(),
            "bgfx color program ready",
            outStatus);
    }

    bool BgfxShaderProgram::loadTextureProgram(std::string& outStatus)
    {
        return loadProgram(
            RenderAssetPaths::textureVertexShaderPath(),
            RenderAssetPaths::textureFragmentShaderPath(),
            "bgfx texture program ready",
            outStatus);
    }

    bool BgfxShaderProgram::loadProgram(
        const std::string& vertexShaderPath,
        const std::string& fragmentShaderPath,
        const char* readyMessage,
        std::string& outStatus)
    {
#if WAR_HAS_BGFX
        if (m_attemptedLoad)
        {
            outStatus = m_lastStatus;
            return m_ready;
        }

        m_attemptedLoad = true;

        if (vertexShaderPath.empty() || fragmentShaderPath.empty())
        {
            m_lastStatus = "unsupported bgfx renderer for shader asset folder";
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::Memory* vsMemory = loadMemoryFromFile(vertexShaderPath.c_str());
        if (vsMemory == nullptr)
        {
            m_lastStatus = "missing shader binary: " + vertexShaderPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::Memory* fsMemory = loadMemoryFromFile(fragmentShaderPath.c_str());
        if (fsMemory == nullptr)
        {
            m_lastStatus = "missing shader binary: " + fragmentShaderPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::ShaderHandle vs = bgfx::createShader(vsMemory);
        if (!bgfx::isValid(vs))
        {
            m_lastStatus = "failed to create bgfx vertex shader: " + vertexShaderPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::ShaderHandle fs = bgfx::createShader(fsMemory);
        if (!bgfx::isValid(fs))
        {
            bgfx::destroy(vs);
            m_lastStatus = "failed to create bgfx fragment shader: " + fragmentShaderPath;
            outStatus = m_lastStatus;
            return false;
        }

        m_program = bgfx::createProgram(vs, fs, true);
        if (!bgfx::isValid(m_program))
        {
            bgfx::destroy(vs);
            bgfx::destroy(fs);
            m_lastStatus = "failed to create bgfx shader program";
            outStatus = m_lastStatus;
            return false;
        }

        m_ready = true;
        m_lastStatus = readyMessage;
        outStatus = m_lastStatus;
        return true;
#else
        (void)vertexShaderPath;
        (void)fragmentShaderPath;
        (void)readyMessage;
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
        m_lastStatus = "bgfx shader program not loaded";
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
