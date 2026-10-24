/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** FixedTimestep.cpp
*/

#include "Engine/Core/FixedTimestep.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <limits>
#include <stdexcept>

using Catch::Approx;
using engine::core::FixedTimestep;

TEST_CASE("less than one step yields no step", "[core][timestep]")
{
    FixedTimestep timestep(0.25, 5);

    REQUIRE(timestep.advance(0.125) == 0);
    REQUIRE(timestep.alpha() == Approx(0.5));
}

TEST_CASE("whole steps are consumed and the remainder kept", "[core][timestep]")
{
    FixedTimestep timestep(0.25, 10);

    REQUIRE(timestep.advance(0.625) == 2);
    REQUIRE(timestep.alpha() == Approx(0.5));
    REQUIRE(timestep.advance(0.125) == 1);
    REQUIRE(timestep.alpha() == Approx(0.0));
}

TEST_CASE("the step count is capped and the excess dropped", "[core][timestep]")
{
    FixedTimestep timestep(0.25, 3);

    REQUIRE(timestep.advance(10.125) == 3);
    REQUIRE(timestep.alpha() == Approx(0.5));
    REQUIRE(timestep.advance(0.0) == 0);
}

TEST_CASE("the result does not depend on how time is split", "[core][timestep]")
{
    FixedTimestep whole(0.25, 100);
    FixedTimestep split(0.25, 100);
    std::size_t total = 0;

    for (int i = 0; i < 20; ++i)
        total += split.advance(0.125);
    REQUIRE(total == whole.advance(2.5));
    REQUIRE(split.alpha() == Approx(whole.alpha()));
}

TEST_CASE("an invalid configuration is rejected", "[core][timestep]")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();

    REQUIRE_THROWS_AS(FixedTimestep(0.0, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(FixedTimestep(-0.1, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(FixedTimestep(nan, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(FixedTimestep(0.1, 0), std::invalid_argument);
}

TEST_CASE("an invalid elapsed time is rejected", "[core][timestep]")
{
    FixedTimestep timestep(0.25, 5);
    const double inf = std::numeric_limits<double>::infinity();

    REQUIRE_THROWS_AS(timestep.advance(-0.1), std::invalid_argument);
    REQUIRE_THROWS_AS(timestep.advance(inf), std::invalid_argument);
    REQUIRE(timestep.advance(0.25) == 1);
}

TEST_CASE("the configuration is exposed", "[core][timestep]")
{
    FixedTimestep timestep(0.5, 7);

    REQUIRE(timestep.step() == Approx(0.5));
    REQUIRE(timestep.maxSteps() == 7);
}
