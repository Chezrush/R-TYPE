/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** OffScreen.cpp
*/

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <variant>

#include "Rtype/Simulation.hpp"

using Catch::Approx;
using engine::core::Vec2;
using rtype::game::DestroyEvent;
using rtype::game::DestroyReason;
using rtype::game::EntityKind;
using rtype::game::EntityState;
using rtype::game::GameEvent;
using rtype::game::GameRules;
using rtype::game::NO_PLAYER;
using rtype::game::Simulation;
using rtype::game::SpawnEvent;
namespace keys = rtype::game::keys;

namespace {

GameRules quiet()
{
    GameRules rules;

    rules.enemySpawnInterval = 0.0F;
    return rules;
}

constexpr std::uint32_t SEED = 3;
constexpr std::uint8_t SLOT = 0;

GameRules smallWorld()
{
    GameRules rules = quiet();
    rules.worldWidth = 200.0F;
    return rules;
}

Simulation shooter(const GameRules &rules)
{
    Simulation simulation(rules, SEED);

    REQUIRE(simulation.addPlayer(SLOT));
    simulation.setInput(SLOT, keys::SHOOT);
    simulation.step();
    simulation.setInput(SLOT, 0);
    return simulation;
}

std::size_t countOf(const Simulation &simulation, EntityKind kind)
{
    std::size_t count = 0;

    for (const EntityState &state : simulation.states()) {
        count += (state.kind == kind) ? 1U : 0U;
    }
    return count;
}

std::vector<DestroyEvent> destroysIn(const std::vector<GameEvent> &events)
{
    std::vector<DestroyEvent> found;

    for (const GameEvent &event : events) {
        if (const auto *destroy = std::get_if<DestroyEvent>(&event)) {
            found.push_back(*destroy);
        }
    }
    return found;
}

std::optional<std::uint16_t> missileId(const std::vector<GameEvent> &events)
{
    for (const GameEvent &event : events) {
        const auto *spawn = std::get_if<SpawnEvent>(&event);

        if (spawn != nullptr && spawn->kind == EntityKind::PLAYER_MISSILE) {
            return spawn->netId;
        }
    }
    return std::nullopt;
}

std::uint64_t firstDestroyTick(Simulation &simulation, int limit)
{
    for (int i = 0; i < limit; ++i) {
        simulation.step();
        if (!destroysIn(simulation.takeEvents()).empty()) {
            return simulation.tick();
        }
    }
    return 0;
}

} // namespace

TEST_CASE("a step lasts exactly one over the tick rate", "[game][movement]")
{
    for (const std::uint32_t tickRate : {30U, 60U, 120U}) {
        GameRules rules = quiet();
        rules.tickRate = tickRate;
        Simulation simulation(rules, SEED);

        REQUIRE(simulation.addPlayer(SLOT));
        const float before = simulation.states().front().position.x;
        simulation.setInput(SLOT, keys::RIGHT);
        simulation.step();

        REQUIRE(simulation.states().front().position.x - before ==
                Approx(rules.shipSpeed / static_cast<float>(tickRate))
                    .margin(0.001));
    }
}

TEST_CASE("a missile still on screen is not destroyed", "[game][bounds]")
{
    Simulation simulation = shooter(quiet());

    for (int i = 0; i < 50; ++i) {
        simulation.step();
    }
    REQUIRE(countOf(simulation, EntityKind::PLAYER_MISSILE) == 1);
    REQUIRE(destroysIn(simulation.takeEvents()).empty());
}

TEST_CASE("a missile that leaves the screen is destroyed", "[game][bounds]")
{
    Simulation simulation = shooter(quiet());
    const auto id = missileId(simulation.takeEvents());

    for (int i = 0; i < 200; ++i) {
        simulation.step();
    }
    const auto destroys = destroysIn(simulation.takeEvents());

    REQUIRE(id.has_value());
    REQUIRE(destroys.size() == 1);
    REQUIRE(destroys[0].netId == *id);
    REQUIRE(destroys[0].reason == DestroyReason::LEFT_SCREEN);
    REQUIRE(destroys[0].killer == NO_PLAYER);
    REQUIRE(countOf(simulation, EntityKind::PLAYER_MISSILE) == 0);
}

TEST_CASE("a missile is destroyed on the tick it is entirely outside",
          "[game][bounds]")
{
    Simulation simulation = shooter(smallWorld());

    static_cast<void>(simulation.takeEvents());
    REQUIRE(firstDestroyTick(simulation, 100) == 15);
}

TEST_CASE("a destroyed missile is not destroyed again", "[game][bounds]")
{
    Simulation simulation = shooter(smallWorld());
    std::size_t destroys = 0;

    for (int i = 0; i < 100; ++i) {
        simulation.step();
        destroys += destroysIn(simulation.takeEvents()).size();
    }
    REQUIRE(destroys == 1);
}

TEST_CASE("a ship at the edge of the world is never destroyed",
          "[game][bounds]")
{
    Simulation simulation(quiet(), SEED);

    REQUIRE(simulation.addPlayer(SLOT));
    simulation.setInput(SLOT, keys::RIGHT | keys::DOWN);
    for (int i = 0; i < 1000; ++i) {
        simulation.step();
    }
    REQUIRE(countOf(simulation, EntityKind::PLAYER_SHIP) == 1);
    REQUIRE(
        simulation.states().front().position ==
        Vec2{simulation.rules().worldWidth, simulation.rules().worldHeight});
}

TEST_CASE("the same game destroys the same entities on the same ticks",
          "[game][bounds]")
{
    Simulation first = shooter(smallWorld());
    Simulation second = shooter(smallWorld());

    static_cast<void>(first.takeEvents());
    static_cast<void>(second.takeEvents());

    REQUIRE(firstDestroyTick(first, 100) == firstDestroyTick(second, 100));
}
