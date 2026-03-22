#include "engine/render/BgfxTextureAsset.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include "../../../../third_party/bimg/3rdparty/lodepng/lodepng.h"
// Keep PNG decode self-contained in the renderer build without adding project-file entries.
#include "../../../../third_party/bimg/3rdparty/lodepng/lodepng.cpp"

namespace war
{
#if WAR_HAS_BGFX
    namespace
    {
#pragma pack(push, 1)
        struct BmpFileHeader
        {
            uint16_t type = 0;
            uint32_t size = 0;
            uint16_t reserved1 = 0;
            uint16_t reserved2 = 0;
            uint32_t offBits = 0;
        };

        struct BmpInfoHeader
        {
            uint32_t size = 0;
            int32_t width = 0;
            int32_t height = 0;
            uint16_t planes = 0;
            uint16_t bitCount = 0;
            uint32_t compression = 0;
            uint32_t sizeImage = 0;
            int32_t xPelsPerMeter = 0;
            int32_t yPelsPerMeter = 0;
            uint32_t clrUsed = 0;
            uint32_t clrImportant = 0;
        };
#pragma pack(pop)

        struct TexturePixels
        {
            uint32_t width = 0;
            uint32_t height = 0;
            std::vector<uint8_t> bgra8;
        };

        bool loadBmpPixels(const std::string& path, TexturePixels& outPixels, std::string& outStatus)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
            {
                outStatus = "missing texture asset: " + path;
                return false;
            }

            BmpFileHeader fileHeader{};
            BmpInfoHeader infoHeader{};

            file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
            file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

            if (!file)
            {
                outStatus = "failed to read bmp headers: " + path;
                return false;
            }

            if (fileHeader.type != 0x4d42)
            {
                outStatus = "unsupported bmp signature: " + path;
                return false;
            }

            if (infoHeader.size != sizeof(BmpInfoHeader))
            {
                outStatus = "unsupported bmp header size: " + path;
                return false;
            }

            if (infoHeader.width <= 0 || infoHeader.height == 0)
            {
                outStatus = "invalid bmp dimensions: " + path;
                return false;
            }

            if (infoHeader.planes != 1)
            {
                outStatus = "invalid bmp planes: " + path;
                return false;
            }

            if (infoHeader.compression != 0)
            {
                outStatus = "compressed bmp textures are not supported: " + path;
                return false;
            }

            if (infoHeader.bitCount != 24 && infoHeader.bitCount != 32)
            {
                outStatus = "only 24-bit and 32-bit bmp textures are supported: " + path;
                return false;
            }

            const uint32_t width = static_cast<uint32_t>(infoHeader.width);
            const uint32_t height = static_cast<uint32_t>(infoHeader.height > 0 ? infoHeader.height : -infoHeader.height);
            const bool bottomUp = infoHeader.height > 0;

            const uint32_t bytesPerPixel = static_cast<uint32_t>(infoHeader.bitCount / 8);
            const uint32_t rowStride = ((width * static_cast<uint32_t>(infoHeader.bitCount) + 31u) / 32u) * 4u;

            file.seekg(static_cast<std::streamoff>(fileHeader.offBits), std::ios::beg);
            if (!file)
            {
                outStatus = "failed to seek bmp pixel data: " + path;
                return false;
            }

            std::vector<uint8_t> row(rowStride);
            outPixels.width = width;
            outPixels.height = height;
            outPixels.bgra8.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u);

            for (uint32_t y = 0; y < height; ++y)
            {
                file.read(reinterpret_cast<char*>(row.data()), static_cast<std::streamsize>(rowStride));
                if (!file)
                {
                    outStatus = "failed to read bmp pixel row: " + path;
                    return false;
                }

                const uint32_t destY = bottomUp ? (height - 1u - y) : y;
                uint8_t* dest = outPixels.bgra8.data()
                    + static_cast<size_t>(destY) * static_cast<size_t>(width) * 4u;

                for (uint32_t x = 0; x < width; ++x)
                {
                    const uint8_t* src = row.data() + static_cast<size_t>(x) * bytesPerPixel;
                    dest[x * 4u + 0u] = src[0];
                    dest[x * 4u + 1u] = src[1];
                    dest[x * 4u + 2u] = src[2];
                    dest[x * 4u + 3u] = bytesPerPixel == 4u ? src[3] : 255u;
                }
            }

            outStatus = "bmp texture decoded";
            return true;
        }

        bool loadPngPixels(const std::string& path, TexturePixels& outPixels, std::string& outStatus)
        {
            std::vector<unsigned char> rgba8;
            unsigned width = 0;
            unsigned height = 0;
            const unsigned error = lodepng::decode(rgba8, width, height, path);
            if (error != 0)
            {
                outStatus = "failed to decode png texture: " + path;
                const char* failureReason = lodepng_error_text(error);
                if (failureReason && *failureReason)
                {
                    outStatus += " (" + std::string(failureReason) + ")";
                }

                return false;
            }

            if (width == 0u || height == 0u)
            {
                outStatus = "invalid png dimensions: " + path;
                return false;
            }

            outPixels.width = width;
            outPixels.height = height;
            outPixels.bgra8.resize(static_cast<size_t>(outPixels.width) * static_cast<size_t>(outPixels.height) * 4u);

            const size_t pixelCount = static_cast<size_t>(outPixels.width) * static_cast<size_t>(outPixels.height);
            for (size_t index = 0; index < pixelCount; ++index)
            {
                const size_t sourceOffset = index * 4u;
                const size_t destOffset = index * 4u;
                outPixels.bgra8[destOffset + 0u] = rgba8[sourceOffset + 2u];
                outPixels.bgra8[destOffset + 1u] = rgba8[sourceOffset + 1u];
                outPixels.bgra8[destOffset + 2u] = rgba8[sourceOffset + 0u];
                outPixels.bgra8[destOffset + 3u] = rgba8[sourceOffset + 3u];
            }

            outStatus = "png texture decoded";
            return true;
        }

        bool loadTexturePixels(const std::string& path, TexturePixels& outPixels, std::string& outStatus)
        {
            std::string extension = std::filesystem::path(path).extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char value) {
                return static_cast<char>(std::tolower(value));
            });

            if (extension == ".png")
            {
                return loadPngPixels(path, outPixels, outStatus);
            }

            return loadBmpPixels(path, outPixels, outStatus);
        }
    }
#endif

    BgfxTextureAsset::~BgfxTextureAsset()
    {
        shutdown();
    }

    bool BgfxTextureAsset::loadFromFile(const std::string& path, std::string& outStatus)
    {
#if WAR_HAS_BGFX
        if (m_attemptedLoad)
        {
            outStatus = m_lastStatus;
            return m_ready;
        }

        m_attemptedLoad = true;

        TexturePixels pixels{};
        if (!loadTexturePixels(path, pixels, m_lastStatus))
        {
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::Memory* memory = bgfx::copy(
            pixels.bgra8.data(),
            static_cast<uint32_t>(pixels.bgra8.size()));

        m_texture = bgfx::createTexture2D(
            static_cast<uint16_t>(pixels.width),
            static_cast<uint16_t>(pixels.height),
            false,
            1,
            bgfx::TextureFormat::BGRA8,
            BGFX_TEXTURE_NONE
                | BGFX_SAMPLER_U_CLAMP
                | BGFX_SAMPLER_V_CLAMP
                | BGFX_SAMPLER_MIN_POINT
                | BGFX_SAMPLER_MAG_POINT,
            memory);

        if (!bgfx::isValid(m_texture))
        {
            m_lastStatus = "failed to create bgfx texture: " + path;
            outStatus = m_lastStatus;
            return false;
        }

        m_ready = true;
        m_lastStatus = "bgfx texture ready: " + path;
        outStatus = m_lastStatus;
        return true;
#else
        (void)path;
        m_lastStatus = "bgfx headers not available at compile time";
        outStatus = m_lastStatus;
        return false;
#endif
    }

    void BgfxTextureAsset::shutdown()
    {
#if WAR_HAS_BGFX
        if (bgfx::isValid(m_texture))
        {
            bgfx::destroy(m_texture);
            m_texture = BGFX_INVALID_HANDLE;
        }
#endif

        m_ready = false;
        m_attemptedLoad = false;
        m_lastStatus = "bgfx texture not loaded";
    }

    bool BgfxTextureAsset::isReady() const
    {
        return m_ready;
    }

    const std::string& BgfxTextureAsset::statusMessage() const
    {
        return m_lastStatus;
    }

#if WAR_HAS_BGFX
    bgfx::TextureHandle BgfxTextureAsset::handle() const
    {
        return m_texture;
    }
#endif
}
