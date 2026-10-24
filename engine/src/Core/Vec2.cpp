/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Vec2.cpp
*/

#include "Engine/Core/Vec2.hpp"

#include <cmath>

namespace engine::core {

Vec2 &Vec2::operator+=(const Vec2 &other) noexcept
{
    x += other.x;
    y += other.y;
    return *this;
}

Vec2 &Vec2::operator-=(const Vec2 &other) noexcept
{
    x -= other.x;
    y -= other.y;
    return *this;
}

Vec2 &Vec2::operator*=(float scalar) noexcept
{
    x *= scalar;
    y *= scalar;
    return *this;
}

Vec2 &Vec2::operator/=(float scalar) noexcept
{
    x /= scalar;
    y /= scalar;
    return *this;
}

Vec2 operator+(Vec2 lhs, const Vec2 &rhs) noexcept
{
    return lhs += rhs;
}

Vec2 operator-(Vec2 lhs, const Vec2 &rhs) noexcept
{
    return lhs -= rhs;
}

Vec2 operator-(const Vec2 &vector) noexcept
{
    return {-vector.x, -vector.y};
}

Vec2 operator*(Vec2 vector, float scalar) noexcept
{
    return vector *= scalar;
}

Vec2 operator*(float scalar, Vec2 vector) noexcept
{
    return vector *= scalar;
}

Vec2 operator/(Vec2 vector, float scalar) noexcept
{
    return vector /= scalar;
}

float dot(const Vec2 &lhs, const Vec2 &rhs) noexcept
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

float lengthSquared(const Vec2 &vector) noexcept
{
    return dot(vector, vector);
}

float length(const Vec2 &vector) noexcept
{
    return std::hypot(vector.x, vector.y);
}

Vec2 normalized(const Vec2 &vector) noexcept
{
    const float size = length(vector);

    if (size == 0.0F)
        return {};
    return vector / size;
}

bool nearlyEqual(const Vec2 &lhs, const Vec2 &rhs, float epsilon) noexcept
{
    return std::fabs(lhs.x - rhs.x) <= epsilon &&
           std::fabs(lhs.y - rhs.y) <= epsilon;
}

} // namespace engine::core
