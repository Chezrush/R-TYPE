/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ShipMovement.cpp
*/

#include "Rtype/ShipMovement.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;
using engine::core::length;
using engine::core::Vec2;
using rtype::game::clampToWorld;
using rtype::game::GameRules;
using rtype::game::moveShip;
using rtype::game::shipVelocity;
namespace keys = rtype::game::keys;

namespace {

constexpr float SPEED = 300.0F;

} // namespace

TEST_CASE("no key means no movement", "[game][movement]")
{
    REQUIRE(shipVelocity(0, SPEED) == Vec2{0.0F, 0.0F});
    REQUIRE(shipVelocity(keys::SHOOT, SPEED) == Vec2{0.0F, 0.0F});
}

TEST_CASE("each direction key moves along its axis", "[game][movement]")
{
    REQUIRE(shipVelocity(keys::RIGHT, SPEED) == Vec2{SPEED, 0.0F});
    REQUIRE(shipVelocity(keys::LEFT, SPEED) == Vec2{-SPEED, 0.0F});
    REQUIRE(shipVelocity(keys::DOWN, SPEED) == Vec2{0.0F, SPEED});
    REQUIRE(shipVelocity(keys::UP, SPEED) == Vec2{0.0F, -SPEED});
}

TEST_CASE("opposite keys cancel each other", "[game][movement]")
{
    REQUIRE(shipVelocity(keys::LEFT | keys::RIGHT, SPEED) == Vec2{});
    REQUIRE(shipVelocity(keys::UP | keys::DOWN, SPEED) == Vec2{});
    REQUIRE(shipVelocity(keys::UP | keys::DOWN | keys::LEFT | keys::RIGHT,
                         SPEED) == Vec2{});
    REQUIRE(shipVelocity(keys::UP | keys::DOWN | keys::RIGHT, SPEED) ==
            Vec2{SPEED, 0.0F});
}

TEST_CASE("a diagonal is not faster than a straight line", "[game][movement]")
{
    const Vec2 diagonal = shipVelocity(keys::UP | keys::RIGHT, SPEED);

    REQUIRE(length(diagonal) == Approx(SPEED));
    REQUIRE(diagonal.x > 0.0F);
    REQUIRE(diagonal.y < 0.0F);
    REQUIRE(diagonal.x == Approx(-diagonal.y));
}

TEST_CASE("a position inside the world is not changed", "[game][movement]")
{
    const GameRules rules;

    REQUIRE(clampToWorld({100.0F, 200.0F}, rules) == Vec2{100.0F, 200.0F});
    REQUIRE(clampToWorld({0.0F, rules.worldHeight}, rules) ==
            Vec2{0.0F, rules.worldHeight});
}

TEST_CASE("a position outside the world is moved back to the edge",
          "[game][movement]")
{
    const GameRules rules;

    REQUIRE(clampToWorld({-5.0F, 10.0F}, rules) == Vec2{0.0F, 10.0F});
    REQUIRE(clampToWorld({10.0F, -5.0F}, rules) == Vec2{10.0F, 0.0F});
    REQUIRE(clampToWorld({rules.worldWidth + 1.0F, rules.worldHeight + 1.0F},
                         rules) == Vec2{rules.worldWidth, rules.worldHeight});
}

TEST_CASE("moveShip adds the velocity during the step", "[game][movement]")
{
    const GameRules rules;
    const Vec2 start{100.0F, 100.0F};

    REQUIRE(moveShip(start, keys::RIGHT, rules, 0.5F) ==
            Vec2{100.0F + rules.shipSpeed * 0.5F, 100.0F});
    REQUIRE(moveShip(start, 0, rules, 0.5F) == start);
}

TEST_CASE("moveShip keeps the ship inside the world", "[game][movement]")
{
    const GameRules rules;

    REQUIRE(moveShip({1.0F, 1.0F}, keys::LEFT | keys::UP, rules, 1.0F) ==
            Vec2{0.0F, 0.0F});
    REQUIRE(moveShip({rules.worldWidth, 1.0F}, keys::RIGHT, rules, 1.0F) ==
            Vec2{rules.worldWidth, 1.0F});
}
