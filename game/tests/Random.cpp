/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Random.cpp
*/

#include "Rtype/Random.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;
using rtype::game::Random;

TEST_CASE("the generator gives the SplitMix64 reference numbers",
          "[game][random]")
{
    Random random(0);

    REQUIRE(random.nextU32() == 0xE220A839U);
    REQUIRE(random.nextU32() == 0x6E789E6AU);
    REQUIRE(random.nextU32() == 0x06C45D18U);
}

TEST_CASE("the same seed gives the same numbers", "[game][random]")
{
    Random first(1234);
    Random second(1234);

    for (int i = 0; i < 100; ++i) {
        REQUIRE(first.nextU32() == second.nextU32());
    }
}

TEST_CASE("different seeds give different numbers", "[game][random]")
{
    Random first(1);
    Random second(2);
    int same = 0;

    for (int i = 0; i < 10; ++i) {
        same += (first.nextU32() == second.nextU32()) ? 1 : 0;
    }
    REQUIRE(same == 0);
}

TEST_CASE("a float stays in its range and covers it", "[game][random]")
{
    Random random(99);
    float lowest = 100.0F;
    float highest = -100.0F;
    double sum = 0.0;

    for (int i = 0; i < 10000; ++i) {
        const float value = random.nextFloat(-5.0F, 15.0F);
        REQUIRE(value >= -5.0F);
        REQUIRE(value < 15.0F);
        lowest = (value < lowest) ? value : lowest;
        highest = (value > highest) ? value : highest;
        sum += value;
    }
    REQUIRE(lowest < -4.0F);
    REQUIRE(highest > 14.0F);
    REQUIRE(sum / 10000.0 == Approx(5.0).margin(0.5));
}

TEST_CASE("an empty range always gives its only value", "[game][random]")
{
    Random random(5);

    REQUIRE(random.nextFloat(3.0F, 3.0F) == 3.0F);
}

TEST_CASE("coin tosses give both sides about equally", "[game][random]")
{
    Random random(7);
    int heads = 0;

    for (int i = 0; i < 1000; ++i) {
        heads += random.nextBool() ? 1 : 0;
    }
    REQUIRE(heads > 400);
    REQUIRE(heads < 600);
}
