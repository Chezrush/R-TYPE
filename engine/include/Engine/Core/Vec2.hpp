/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Vec2.hpp
*/

#ifndef VEC2_HPP_
#define VEC2_HPP_

namespace engine::core {

/**
 * @brief Two-dimensional vector of single-precision floats.
 *
 * Used for positions and velocities. Operators follow IEEE 754: dividing by
 * zero gives infinities or NaN, it never throws. Durations are not part of
 * this type, use std::chrono.
 *
 * Equality strategy: operator== is exact and compares both components, so
 * it is only meaningful for values that were copied or computed exactly.
 * Use nearlyEqual() to compare the results of floating-point computations.
 */
struct Vec2 {
    /** @brief Horizontal component. */
    float x = 0.0F;

    /** @brief Vertical component. */
    float y = 0.0F;

    friend bool operator==(const Vec2 &, const Vec2 &) = default;
};

/** @name Component-wise arithmetic, scalar operations scale both axes. */
///@{
Vec2 &operator+=(Vec2 &lhs, const Vec2 &rhs) noexcept;
Vec2 &operator-=(Vec2 &lhs, const Vec2 &rhs) noexcept;
Vec2 &operator*=(Vec2 &vector, float scalar) noexcept;
Vec2 &operator/=(Vec2 &vector, float scalar) noexcept;

[[nodiscard]] Vec2 operator+(Vec2 lhs, const Vec2 &rhs) noexcept;
[[nodiscard]] Vec2 operator-(Vec2 lhs, const Vec2 &rhs) noexcept;
[[nodiscard]] Vec2 operator-(const Vec2 &vector) noexcept;
[[nodiscard]] Vec2 operator*(Vec2 vector, float scalar) noexcept;
[[nodiscard]] Vec2 operator*(float scalar, Vec2 vector) noexcept;
[[nodiscard]] Vec2 operator/(Vec2 vector, float scalar) noexcept;
///@}

/** @return The dot product of the two vectors. */
[[nodiscard]] float dot(const Vec2 &lhs, const Vec2 &rhs) noexcept;

/** @return The squared length, cheaper than length() for comparisons. */
[[nodiscard]] float lengthSquared(const Vec2 &vector) noexcept;

/** @return The Euclidean length, without intermediate overflow. */
[[nodiscard]] float length(const Vec2 &vector) noexcept;

/**
 * @return The vector scaled to a length of 1, or the zero vector when the
 * length is zero, so the result is always safe to use.
 */
[[nodiscard]] Vec2 normalized(const Vec2 &vector) noexcept;

/**
 * @brief Compares two vectors with an absolute tolerance.
 *
 * Each component must differ by at most epsilon. The tolerance is absolute,
 * so it suits values of the magnitude of a screen or a world coordinate, not
 * values spanning many orders of magnitude.
 *
 * @param epsilon Largest accepted difference per component.
 */
[[nodiscard]] bool nearlyEqual(const Vec2 &lhs, const Vec2 &rhs,
                               float epsilon = 1e-5F) noexcept;

} // namespace engine::core

#endif /* !VEC2_HPP_ */
