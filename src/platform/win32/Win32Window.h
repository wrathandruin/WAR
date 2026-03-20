#pragma once

#include <windows.h>

namespace war
{
    class Win32Window
    {
    public:
        bool create(int width, int height, const wchar_t* title);
        void pollEvents();
        bool shouldClose() const;

        [[nodiscard]] HWND getHandle() const;
        [[nodiscard]] int getWidth() const;
        [[nodiscard]] int getHeight() const;

        [[nodiscard]] POINT getMousePosition() const;
        [[nodiscard]] bool consumeLeftClick(POINT& outPoint);
        [[nodiscard]] int consumeMouseWheelDelta();
        [[nodiscard]] bool isMiddleMouseDown() const;
        [[nodiscard]] POINT consumeMouseDelta();

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
        int m_mouseWheelDelta = 0;
        bool m_middleMouseDown = false;
        POINT m_lastMousePosition{};
        POINT m_mouseDelta{};
    };
}
