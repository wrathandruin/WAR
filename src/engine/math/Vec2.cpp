#include "engine/math/Vec2.h"

#include <cmath>

namespace war
{
    Vec2& Vec2::operator+=(const Vec2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vec2& Vec2::operator-=(const Vec2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vec2& Vec2::operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2 operator+(Vec2 lhs, const Vec2& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    Vec2 operator-(Vec2 lhs, const Vec2& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    Vec2 operator*(Vec2 lhs, float scalar)
    {
        lhs *= scalar;
        return lhs;
    }

    float length(const Vec2& value)
    {
        return std::sqrt(value.x * value.x + value.y * value.y);
    }

    Vec2 normalize(const Vec2& value)
    {
        const float len = length(value);
        if (len <= 0.0001f)
        {
            return {};
        }

        return { value.x / len, value.y / len };
    }
}
