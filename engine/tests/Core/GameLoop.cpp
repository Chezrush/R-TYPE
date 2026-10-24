/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameLoop.cpp
*/

#include "Engine/Core/GameLoop.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <thread>

using Catch::Approx;
using engine::core::FixedTimestep;
using engine::core::GameLoop;

namespace {

struct Fixture {
    double now = 0.0;
    std::size_t steps = 0;
    double lastStep = 0.0;

    GameLoop makeLoop(std::size_t maxSteps = 100)
    {
        return GameLoop(
            FixedTimestep(0.25, maxSteps), [this] { return now; },
            [this](double step) {
                ++steps;
                lastStep = step;
            });
    }
};

} // namespace

TEST_CASE("tick runs one step per due fixed step", "[core][loop]")
{
    Fixture fixture;
    GameLoop loop = fixture.makeLoop();

    fixture.now = 0.625;
    loop.tick();
    REQUIRE(fixture.steps == 2);
    REQUIRE(fixture.lastStep == Approx(0.25));
    REQUIRE(loop.alpha() == Approx(0.5));
}

TEST_CASE("the step count does not depend on the frame rate", "[core][loop]")
{
    Fixture slow;
    Fixture fast;
    GameLoop slowLoop = slow.makeLoop();
    GameLoop fastLoop = fast.makeLoop();

    slow.now = 1.25;
    slowLoop.tick();
    for (int i = 1; i <= 10; ++i) {
        fast.now = 0.125 * i;
        fastLoop.tick();
    }
    REQUIRE(slow.steps == 5);
    REQUIRE(fast.steps == slow.steps);
}

TEST_CASE("a long stall is capped", "[core][loop]")
{
    Fixture fixture;
    GameLoop loop = fixture.makeLoop(3);

    fixture.now = 60.0;
    loop.tick();
    REQUIRE(fixture.steps == 3);
}

TEST_CASE("a clock going backwards yields no step", "[core][loop]")
{
    Fixture fixture;
    fixture.now = 5.0;
    GameLoop loop = fixture.makeLoop();

    fixture.now = 4.0;
    loop.tick();
    REQUIRE(fixture.steps == 0);
    fixture.now = 4.25;
    loop.tick();
    REQUIRE(fixture.steps == 1);
}

TEST_CASE("an empty callback is rejected", "[core][loop]")
{
    FixedTimestep timestep(0.25, 1);

    REQUIRE_THROWS_AS(GameLoop(timestep, nullptr, [](double) {}),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(
        GameLoop(timestep, [] { return 0.0; }, nullptr), std::invalid_argument);
}

TEST_CASE("run returns once the step requests a stop", "[core][loop]")
{
    double now = 0.0;
    std::size_t steps = 0;
    GameLoop *self = nullptr;
    GameLoop loop(
        FixedTimestep(0.25, 10), [&now] { return now += 0.25; },
        [&](double) {
            if (++steps == 3)
                self->stop();
        });

    self = &loop;
    loop.run();
    REQUIRE(steps == 3);
}

TEST_CASE("run can start again after a stop", "[core][loop]")
{
    double now = 0.0;
    std::size_t steps = 0;
    GameLoop *self = nullptr;
    GameLoop loop(
        FixedTimestep(0.25, 10), [&now] { return now += 0.25; },
        [&](double) {
            ++steps;
            self->stop();
        });

    self = &loop;
    loop.run();
    loop.run();
    REQUIRE(steps == 2);
}

TEST_CASE("stop works from another thread", "[core][loop]")
{
    using std::chrono::steady_clock;
    const auto origin = steady_clock::now();
    GameLoop loop(
        FixedTimestep(0.01, 10),
        [origin] {
            return std::chrono::duration<double>(steady_clock::now() - origin)
                .count();
        },
        [](double) {});

    std::thread stopper([&loop] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        loop.stop();
    });
    loop.run();
    stopper.join();
    SUCCEED();
}
