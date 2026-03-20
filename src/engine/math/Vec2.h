#pragma once

namespace war
{
    struct Vec2
    {
        float x = 0.0f;
        float y = 0.0f;

        Vec2& operator+=(const Vec2& rhs);
        Vec2& operator-=(const Vec2& rhs);
        Vec2& operator*=(float scalar);
    };

    Vec2 operator+(Vec2 lhs, const Vec2& rhs);
    Vec2 operator-(Vec2 lhs, const Vec2& rhs);
    Vec2 operator*(Vec2 lhs, float scalar);

    [[nodiscard]] float length(const Vec2& value);
    [[nodiscard]] Vec2 normalize(const Vec2& value);
}
