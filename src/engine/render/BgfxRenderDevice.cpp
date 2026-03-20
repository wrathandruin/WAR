#include "engine/render/BgfxRenderDevice.h"

namespace war
{
    bool BgfxRenderDevice::initialize(HWND)
    {
        return false;
    }

    bool BgfxRenderDevice::beginFrame(HWND, const RECT&)
    {
        return false;
    }

    HDC BgfxRenderDevice::getDrawContext() const
    {
        return nullptr;
    }

    void BgfxRenderDevice::endFrame(HWND)
    {
    }

    void BgfxRenderDevice::shutdown()
    {
    }

    const char* BgfxRenderDevice::name() const
    {
        return "bgfx (stub)";
    }
}
