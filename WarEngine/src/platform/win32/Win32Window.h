#pragma once

#include <windows.h>

#include "platform/IWindow.h"

namespace war
{
    class Win32Window final : public IWindow
    {
    public:
        bool create(int width, int height, const wchar_t* title);
        void pollEvents() override;
        [[nodiscard]] bool shouldClose() const override;

        [[nodiscard]] HWND getHandle() const override;
        [[nodiscard]] int getWidth() const override;
        [[nodiscard]] int getHeight() const override;

        [[nodiscard]] POINT getMousePosition() const override;
        [[nodiscard]] bool consumeLeftClick(POINT& outPoint) override;
        [[nodiscard]] bool consumeRightClick(POINT& outPoint) override;
        [[nodiscard]] int consumeMouseWheelDelta() override;
        [[nodiscard]] bool isMiddleMouseDown() const override;
        [[nodiscard]] POINT consumeMouseDelta() override;

    private:
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

        HWND m_hwnd = nullptr;
        int m_width = 0;
        int m_height = 0;
        bool m_shouldClose = false;

        POINT m_mousePosition{};
        bool m_hasPendingLeftClick = false;
        POINT m_pendingLeftClick{};
        bool m_hasPendingRightClick = false;
        POINT m_pendingRightClick{};
        int m_mouseWheelDelta = 0;
        bool m_middleMouseDown = false;
        POINT m_lastMousePosition{};
        POINT m_mouseDelta{};
    };
}
