/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Bounds.cpp
*/

#include "Rtype/Bounds.hpp"

#include <catch2/catch_test_macros.hpp>

using engine::core::Vec2;
using rtype::game::GameRules;
using rtype::game::Hitbox;
using rtype::game::isOutsideWorld;

namespace {

const GameRules RULES;
const Hitbox BOX{20.0F, 10.0F};

} // namespace

TEST_CASE("a rectangle inside the world is not outside", "[game][bounds]")
{
    REQUIRE_FALSE(isOutsideWorld({100.0F, 100.0F}, BOX, RULES));
    REQUIRE_FALSE(isOutsideWorld({0.0F, 0.0F}, BOX, RULES));
    REQUIRE_FALSE(
        isOutsideWorld({RULES.worldWidth, RULES.worldHeight}, BOX, RULES));
}

TEST_CASE("a rectangle partly visible is not outside", "[game][bounds]")
{
    REQUIRE_FALSE(isOutsideWorld({-9.0F, 100.0F}, BOX, RULES));
    REQUIRE_FALSE(
        isOutsideWorld({RULES.worldWidth + 9.0F, 100.0F}, BOX, RULES));
    REQUIRE_FALSE(isOutsideWorld({100.0F, -4.0F}, BOX, RULES));
    REQUIRE_FALSE(
        isOutsideWorld({100.0F, RULES.worldHeight + 4.0F}, BOX, RULES));
}

TEST_CASE("a rectangle that only touches the edge is not outside",
          "[game][bounds]")
{
    REQUIRE_FALSE(isOutsideWorld({-10.0F, 100.0F}, BOX, RULES));
    REQUIRE_FALSE(
        isOutsideWorld({RULES.worldWidth + 10.0F, 100.0F}, BOX, RULES));
    REQUIRE_FALSE(isOutsideWorld({100.0F, -5.0F}, BOX, RULES));
    REQUIRE_FALSE(
        isOutsideWorld({100.0F, RULES.worldHeight + 5.0F}, BOX, RULES));
}

TEST_CASE("a rectangle entirely past an edge is outside", "[game][bounds]")
{
    REQUIRE(isOutsideWorld({-11.0F, 100.0F}, BOX, RULES));
    REQUIRE(isOutsideWorld({RULES.worldWidth + 11.0F, 100.0F}, BOX, RULES));
    REQUIRE(isOutsideWorld({100.0F, -6.0F}, BOX, RULES));
    REQUIRE(isOutsideWorld({100.0F, RULES.worldHeight + 6.0F}, BOX, RULES));
}

TEST_CASE("a point without size is outside as soon as it leaves",
          "[game][bounds]")
{
    const Hitbox point{0.0F, 0.0F};

    REQUIRE_FALSE(isOutsideWorld({0.0F, 0.0F}, point, RULES));
    REQUIRE(isOutsideWorld({-0.5F, 0.0F}, point, RULES));
}
