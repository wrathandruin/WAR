#pragma once

#include <windows.h>

namespace war
{
    class IWindow
    {
    public:
        virtual ~IWindow() = default;

        virtual void pollEvents() = 0;
        [[nodiscard]] virtual bool shouldClose() const = 0;

        [[nodiscard]] virtual HWND getHandle() const = 0;
        [[nodiscard]] virtual int getWidth() const = 0;
        [[nodiscard]] virtual int getHeight() const = 0;

        [[nodiscard]] virtual POINT getMousePosition() const = 0;
        [[nodiscard]] virtual bool consumeLeftClick(POINT& outPoint) = 0;
        [[nodiscard]] virtual bool consumeRightClick(POINT& outPoint) = 0;
        [[nodiscard]] virtual int consumeMouseWheelDelta() = 0;
        [[nodiscard]] virtual bool isMiddleMouseDown() const = 0;
        [[nodiscard]] virtual POINT consumeMouseDelta() = 0;
    };
}
