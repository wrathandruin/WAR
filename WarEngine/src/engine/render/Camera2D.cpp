#include "engine/render/Camera2D.h"

#include <algorithm>

namespace war
{
    void Camera2D::setViewportSize(int width, int height)
    {
        m_viewportWidth = width > 0 ? width : 1;
        m_viewportHeight = height > 0 ? height : 1;
    }

    void Camera2D::setPosition(Vec2 position)
    {
        m_position = position;
    }

    void Camera2D::pan(Vec2 delta)
    {
        m_position += delta;
    }

    void Camera2D::zoomBy(float amount)
    {
        m_zoom = std::clamp(m_zoom + amount, 0.25f, 4.0f);
    }

    Vec2 Camera2D::getPosition() const
    {
        return m_position;
    }

    float Camera2D::getZoom() const
    {
        return m_zoom;
    }

    Vec2 Camera2D::screenToWorld(int screenX, int screenY) const
    {
        const float halfWidth = static_cast<float>(m_viewportWidth) * 0.5f;
        const float halfHeight = static_cast<float>(m_viewportHeight) * 0.5f;

        return {
            m_position.x + (static_cast<float>(screenX) - halfWidth) / m_zoom,
            m_position.y + (static_cast<float>(screenY) - halfHeight) / m_zoom
        };
    }

    Vec2 Camera2D::worldToScreen(const Vec2& world) const
    {
        const float halfWidth = static_cast<float>(m_viewportWidth) * 0.5f;
        const float halfHeight = static_cast<float>(m_viewportHeight) * 0.5f;

        return {
            (world.x - m_position.x) * m_zoom + halfWidth,
            (world.y - m_position.y) * m_zoom + halfHeight
        };
    }
}
