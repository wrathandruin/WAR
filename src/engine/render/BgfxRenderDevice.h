#pragma once

#include "engine/render/IRenderDevice.h"

namespace war
{
    class BgfxRenderDevice final : public IRenderDevice
    {
    public:
        bool initialize(HWND hwnd) override;
        bool beginFrame(HWND hwnd, const RECT& clientRect) override;
        HDC getDrawContext() const override;
        void endFrame(HWND hwnd) override;
        void shutdown() override;

        [[nodiscard]] const char* name() const override;
    };
}
