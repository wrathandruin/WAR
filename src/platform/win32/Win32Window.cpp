#include "platform/win32/Win32Window.h"
#include <windowsx.h>

namespace war
{
    namespace
    {
        constexpr wchar_t kWindowClassName[] = L"WARWindowClass";
    }

    bool Win32Window::create(int width, int height, const wchar_t* title)
    {
        m_width = width;
        m_height = height;

        WNDCLASSW wc{};
        wc.lpfnWndProc = Win32Window::windowProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = kWindowClassName;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        RegisterClassW(&wc);

        RECT rect{ 0, 0, width, height };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hwnd = CreateWindowExW(
            0,
            kWindowClassName,
            title,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            this);

        return m_hwnd != nullptr;
    }

    void Win32Window::pollEvents()
    {
        m_mouseDelta = { 0, 0 };

        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    bool Win32Window::shouldClose() const
    {
        return m_shouldClose;
    }

    HWND Win32Window::getHandle() const
    {
        return m_hwnd;
    }

    int Win32Window::getWidth() const
    {
        return m_width;
    }

    int Win32Window::getHeight() const
    {
        return m_height;
    }

    POINT Win32Window::getMousePosition() const
    {
        return m_mousePosition;
    }

    bool Win32Window::consumeLeftClick(POINT& outPoint)
    {
        if (!m_hasPendingLeftClick)
        {
            return false;
        }

        m_hasPendingLeftClick = false;
        outPoint = m_pendingLeftClick;
        return true;
    }

    int Win32Window::consumeMouseWheelDelta()
    {
        const int value = m_mouseWheelDelta;
        m_mouseWheelDelta = 0;
        return value;
    }

    bool Win32Window::isMiddleMouseDown() const
    {
        return m_middleMouseDown;
    }

    POINT Win32Window::consumeMouseDelta()
    {
        const POINT value = m_mouseDelta;
        m_mouseDelta = { 0, 0 };
        return value;
    }

    LRESULT CALLBACK Win32Window::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32Window* window = nullptr;

        if (msg == WM_NCCREATE)
        {
            const CREATESTRUCTW* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
            window = static_cast<Win32Window*>(create->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            window->m_hwnd = hwnd;
        }
        else
        {
            window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (window != nullptr)
        {
            return window->handleMessage(msg, wParam, lParam);
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT Win32Window::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CLOSE:
            m_shouldClose = true;
            DestroyWindow(m_hwnd);
            return 0;

        case WM_DESTROY:
            m_shouldClose = true;
            PostQuitMessage(0);
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_SIZE:
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            return 0;

        case WM_MOUSEMOVE:
        {
            const POINT newPos{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_mouseDelta.x += newPos.x - m_lastMousePosition.x;
            m_mouseDelta.y += newPos.y - m_lastMousePosition.y;
            m_lastMousePosition = newPos;
            m_mousePosition = newPos;
            return 0;
        }

        case WM_LBUTTONDOWN:
            m_hasPendingLeftClick = true;
            m_pendingLeftClick = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            return 0;

        case WM_MBUTTONDOWN:
            m_middleMouseDown = true;
            SetCapture(m_hwnd);
            return 0;

        case WM_MBUTTONUP:
            m_middleMouseDown = false;
            ReleaseCapture();
            return 0;

        case WM_MOUSEWHEEL:
            m_mouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
            return 0;



        default:
            return DefWindowProcW(m_hwnd, msg, wParam, lParam);
        }
    }
}
