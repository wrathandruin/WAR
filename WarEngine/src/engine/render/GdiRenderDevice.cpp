#include "engine/render/GdiRenderDevice.h"

namespace war
{
    GdiRenderDevice::~GdiRenderDevice()
    {
        shutdown();
    }

    bool GdiRenderDevice::initialize(HWND)
    {
        return true;
    }

    bool GdiRenderDevice::beginFrame(HWND hwnd, const RECT& clientRect)
    {
        m_width = clientRect.right - clientRect.left;
        m_height = clientRect.bottom - clientRect.top;

        if (m_width <= 0 || m_height <= 0)
        {
            return false;
        }

        m_windowDc = GetDC(hwnd);
        if (m_windowDc == nullptr)
        {
            return false;
        }

        m_memoryDc = CreateCompatibleDC(m_windowDc);
        if (m_memoryDc == nullptr)
        {
            ReleaseDC(hwnd, m_windowDc);
            m_windowDc = nullptr;
            return false;
        }

        m_backBuffer = CreateCompatibleBitmap(m_windowDc, m_width, m_height);
        if (m_backBuffer == nullptr)
        {
            DeleteDC(m_memoryDc);
            ReleaseDC(hwnd, m_windowDc);
            m_memoryDc = nullptr;
            m_windowDc = nullptr;
            return false;
        }

        m_oldBitmap = SelectObject(m_memoryDc, m_backBuffer);
        return true;
    }

    HDC GdiRenderDevice::getDrawContext() const
    {
        return m_memoryDc;
    }

    void GdiRenderDevice::endFrame(HWND hwnd)
    {
        if (m_windowDc != nullptr && m_memoryDc != nullptr && m_width > 0 && m_height > 0)
        {
            BitBlt(m_windowDc, 0, 0, m_width, m_height, m_memoryDc, 0, 0, SRCCOPY);
        }

        releaseFrameResources();

        if (m_windowDc != nullptr)
        {
            ReleaseDC(hwnd, m_windowDc);
            m_windowDc = nullptr;
        }
    }

    void GdiRenderDevice::shutdown()
    {
        releaseFrameResources();
    }

    const char* GdiRenderDevice::name() const
    {
        return "GDI";
    }

    void GdiRenderDevice::releaseFrameResources()
    {
        if (m_memoryDc != nullptr && m_oldBitmap != nullptr)
        {
            SelectObject(m_memoryDc, m_oldBitmap);
            m_oldBitmap = nullptr;
        }

        if (m_backBuffer != nullptr)
        {
            DeleteObject(m_backBuffer);
            m_backBuffer = nullptr;
        }

        if (m_memoryDc != nullptr)
        {
            DeleteDC(m_memoryDc);
            m_memoryDc = nullptr;
        }

        m_width = 0;
        m_height = 0;
    }
}
