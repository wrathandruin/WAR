#pragma once

#include "engine/render/IRenderDevice.h"

namespace war
{
    class GdiRenderDevice final : public IRenderDevice
    {
    public:
        ~GdiRenderDevice() override;

        bool initialize(HWND hwnd) override;
        bool beginFrame(HWND hwnd, const RECT& clientRect) override;
        HDC getDrawContext() const override;
        void endFrame(HWND hwnd) override;
        void shutdown() override;

        [[nodiscard]] const char* name() const override;

    private:
        void releaseFrameResources();

        HDC m_windowDc = nullptr;
        HDC m_memoryDc = nullptr;
        HBITMAP m_backBuffer = nullptr;
        HGDIOBJ m_oldBitmap = nullptr;
        int m_width = 0;
        int m_height = 0;
    };
}
