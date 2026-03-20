#include "engine/render/BgfxRenderDevice.h"

#if defined(__has_include)
#  if __has_include(<bgfx/bgfx.h>) && __has_include(<bgfx/platform.h>)
#    define WAR_HAS_BGFX 1
#  else
#    define WAR_HAS_BGFX 0
#  endif
#else
#  define WAR_HAS_BGFX 0
#endif

#if WAR_HAS_BGFX
#  include <bgfx/bgfx.h>
#  include <bgfx/platform.h>
#endif

namespace war
{
    bool BgfxRenderDevice::initialize(HWND hwnd)
    {
#if WAR_HAS_BGFX
        bgfx::Init init{};
        init.type = bgfx::RendererType::Direct3D11;
        init.vendorId = BGFX_PCI_ID_NONE;

        bgfx::PlatformData pd{};
        pd.nwh = hwnd;
        init.platformData = pd;

        RECT rect{};
        GetClientRect(hwnd, &rect);
        m_width = static_cast<unsigned int>(rect.right - rect.left);
        m_height = static_cast<unsigned int>(rect.bottom - rect.top);

        if (m_width == 0) m_width = 1;
        if (m_height == 0) m_height = 1;

        init.resolution.width = m_width;
        init.resolution.height = m_height;
        init.resolution.reset = BGFX_RESET_VSYNC;

        if (!bgfx::init(init))
        {
            m_initialized = false;
            return false;
        }

        bgfx::setDebug(BGFX_DEBUG_TEXT);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x101218ff, 1.0f, 0);
        m_initialized = true;
        return true;
#else
        (void)hwnd;
        m_initialized = false;
        return false;
#endif
    }

    bool BgfxRenderDevice::beginFrame(HWND hwnd, const RECT& clientRect)
    {
#if WAR_HAS_BGFX
        if (!m_initialized)
        {
            return false;
        }

        const unsigned int newWidth = static_cast<unsigned int>(clientRect.right - clientRect.left);
        const unsigned int newHeight = static_cast<unsigned int>(clientRect.bottom - clientRect.top);

        if (newWidth == 0 || newHeight == 0)
        {
            return false;
        }

        if (newWidth != m_width || newHeight != m_height)
        {
            m_width = newWidth;
            m_height = newHeight;
            bgfx::reset(m_width, m_height, BGFX_RESET_VSYNC);
            bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x101218ff, 1.0f, 0);
        }

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
        bgfx::touch(0);
        (void)hwnd;
        return true;
#else
        (void)hwnd;
        (void)clientRect;
        return false;
#endif
    }

    HDC BgfxRenderDevice::getDrawContext() const
    {
        return nullptr;
    }

    void BgfxRenderDevice::endFrame(HWND hwnd)
    {
#if WAR_HAS_BGFX
        if (m_initialized)
        {
            bgfx::frame();
        }
#else
        (void)hwnd;
#endif
        (void)hwnd;
    }

    void BgfxRenderDevice::shutdown()
    {
#if WAR_HAS_BGFX
        if (m_initialized)
        {
            bgfx::shutdown();
        }
#endif
        m_initialized = false;
        m_width = 0;
        m_height = 0;
    }

    const char* BgfxRenderDevice::name() const
    {
#if WAR_HAS_BGFX
        return "bgfx";
#else
        return "bgfx (dependency missing)";
#endif
    }
}
