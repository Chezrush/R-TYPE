/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Vec2.cpp
*/

#include "Engine/Core/Vec2.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <limits>

using Catch::Approx;
using engine::core::dot;
using engine::core::length;
using engine::core::lengthSquared;
using engine::core::nearlyEqual;
using engine::core::normalized;
using engine::core::Vec2;

TEST_CASE("a default vector is the zero vector", "[core][vec2]")
{
    const Vec2 zero;

    REQUIRE(zero.x == 0.0F);
    REQUIRE(zero.y == 0.0F);
}

TEST_CASE("addition and subtraction work component-wise", "[core][vec2]")
{
    const Vec2 a{1.0F, 2.0F};
    const Vec2 b{3.0F, -4.0F};

    REQUIRE(a + b == Vec2{4.0F, -2.0F});
    REQUIRE(a - b == Vec2{-2.0F, 6.0F});
    REQUIRE(-a == Vec2{-1.0F, -2.0F});
}

TEST_CASE("scalar multiplication and division", "[core][vec2]")
{
    const Vec2 a{1.0F, -2.0F};

    REQUIRE(a * 3.0F == Vec2{3.0F, -6.0F});
    REQUIRE(3.0F * a == Vec2{3.0F, -6.0F});
    REQUIRE(a / 2.0F == Vec2{0.5F, -1.0F});
}

TEST_CASE("compound assignments update the vector", "[core][vec2]")
{
    Vec2 a{1.0F, 1.0F};

    a += Vec2{1.0F, 2.0F};
    a -= Vec2{0.0F, 1.0F};
    a *= 2.0F;
    a /= 4.0F;
    REQUIRE(a == Vec2{1.0F, 1.0F});
}

TEST_CASE("division by zero follows IEEE 754", "[core][vec2]")
{
    const Vec2 result = Vec2{1.0F, -1.0F} / 0.0F;

    REQUIRE(result.x == std::numeric_limits<float>::infinity());
    REQUIRE(result.y == -std::numeric_limits<float>::infinity());
}

TEST_CASE("dot product and lengths", "[core][vec2]")
{
    const Vec2 a{3.0F, 4.0F};

    REQUIRE(dot(a, Vec2{2.0F, 1.0F}) == Approx(10.0F));
    REQUIRE(dot(a, Vec2{-4.0F, 3.0F}) == Approx(0.0F));
    REQUIRE(lengthSquared(a) == Approx(25.0F));
    REQUIRE(length(a) == Approx(5.0F));
}

TEST_CASE("length does not overflow on large components", "[core][vec2]")
{
    const Vec2 big{1.0e30F, 1.0e30F};

    REQUIRE(length(big) == Approx(1.41421356e30F));
}

TEST_CASE("normalization gives a unit vector", "[core][vec2]")
{
    const Vec2 unit = normalized(Vec2{3.0F, 4.0F});

    REQUIRE(unit.x == Approx(0.6F));
    REQUIRE(unit.y == Approx(0.8F));
    REQUIRE(length(unit) == Approx(1.0F));
}

TEST_CASE("normalizing the zero vector returns the zero vector", "[core][vec2]")
{
    REQUIRE(normalized(Vec2{}) == Vec2{});
}

TEST_CASE("exact equality compares both components", "[core][vec2]")
{
    REQUIRE(Vec2{1.0F, 2.0F} == Vec2{1.0F, 2.0F});
    REQUIRE(Vec2{1.0F, 2.0F} != Vec2{1.0F, 2.5F});
    REQUIRE(Vec2{1.0F, 2.0F} != Vec2{1.5F, 2.0F});
}

TEST_CASE("nearlyEqual applies an absolute epsilon", "[core][vec2]")
{
    Vec2 sum;

    for (int i = 0; i < 10; ++i)
        sum += Vec2{0.1F, 1.0F};
    REQUIRE_FALSE(sum == Vec2{1.0F, 10.0F});
    REQUIRE(nearlyEqual(sum, Vec2{1.0F, 10.0F}));
    REQUIRE_FALSE(nearlyEqual(sum, Vec2{1.1F, 10.0F}));
    REQUIRE(nearlyEqual(sum, Vec2{1.1F, 10.0F}, 0.2F));
    REQUIRE_FALSE(nearlyEqual(sum, Vec2{1.0F, 10.5F}));
}
