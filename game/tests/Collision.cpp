/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Collision.cpp
*/

#include "Rtype/Collision.hpp"

#include <catch2/catch_test_macros.hpp>

using engine::core::Vec2;
using rtype::game::boxesOverlap;
using rtype::game::Hitbox;

namespace {

const Hitbox BOX{20.0F, 10.0F};

} // namespace

TEST_CASE("boxes at the same place overlap", "[game][collision]")
{
    REQUIRE(boxesOverlap({50.0F, 50.0F}, BOX, {50.0F, 50.0F}, BOX));
}

TEST_CASE("boxes sharing some area overlap", "[game][collision]")
{
    REQUIRE(boxesOverlap({50.0F, 50.0F}, BOX, {59.0F, 54.0F}, BOX));
    REQUIRE(boxesOverlap({50.0F, 50.0F}, BOX, {41.0F, 46.0F}, BOX));
}

TEST_CASE("boxes that only touch by an edge do not overlap",
          "[game][collision]")
{
    REQUIRE_FALSE(boxesOverlap({50.0F, 50.0F}, BOX, {70.0F, 50.0F}, BOX));
    REQUIRE_FALSE(boxesOverlap({50.0F, 50.0F}, BOX, {30.0F, 50.0F}, BOX));
    REQUIRE_FALSE(boxesOverlap({50.0F, 50.0F}, BOX, {50.0F, 60.0F}, BOX));
    REQUIRE_FALSE(boxesOverlap({50.0F, 50.0F}, BOX, {50.0F, 40.0F}, BOX));
}

TEST_CASE("boxes apart on one axis only do not overlap", "[game][collision]")
{
    REQUIRE_FALSE(boxesOverlap({50.0F, 50.0F}, BOX, {51.0F, 80.0F}, BOX));
    REQUIRE_FALSE(boxesOverlap({50.0F, 50.0F}, BOX, {90.0F, 51.0F}, BOX));
}

TEST_CASE("boxes of different sizes use both sizes", "[game][collision]")
{
    const Hitbox big{100.0F, 100.0F};

    REQUIRE(boxesOverlap({0.0F, 0.0F}, big, {59.0F, 0.0F}, BOX));
    REQUIRE_FALSE(boxesOverlap({0.0F, 0.0F}, big, {60.0F, 0.0F}, BOX));
}

TEST_CASE("the overlap does not depend on the order", "[game][collision]")
{
    const Vec2 a{10.0F, 10.0F};
    const Vec2 b{25.0F, 12.0F};

    REQUIRE(boxesOverlap(a, BOX, b, BOX) == boxesOverlap(b, BOX, a, BOX));
}
