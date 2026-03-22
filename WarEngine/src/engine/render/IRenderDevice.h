#pragma once

#include <windows.h>

namespace war
{
    class IRenderDevice
    {
    public:
        virtual ~IRenderDevice() = default;

        virtual bool initialize(HWND hwnd) = 0;
        virtual bool beginFrame(HWND hwnd, const RECT& clientRect) = 0;
        virtual HDC getDrawContext() const = 0;
        virtual void endFrame(HWND hwnd) = 0;
        virtual void shutdown() = 0;

        [[nodiscard]] virtual const char* name() const = 0;
    };
}
