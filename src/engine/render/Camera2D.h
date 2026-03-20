#pragma once

#include "engine/math/Vec2.h"

namespace war
{
    class Camera2D
    {
    public:
        void setViewportSize(int width, int height);
        void setPosition(Vec2 position);
        void pan(Vec2 delta);
        void zoomBy(float amount);

        [[nodiscard]] Vec2 getPosition() const;
        [[nodiscard]] float getZoom() const;

        [[nodiscard]] Vec2 screenToWorld(int screenX, int screenY) const;
        [[nodiscard]] Vec2 worldToScreen(const Vec2& world) const;

    private:
        int m_viewportWidth = 1;
        int m_viewportHeight = 1;
        Vec2 m_position{};
        float m_zoom = 1.0f;
    };
}
