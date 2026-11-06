/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Simulation.cpp
*/

#include "Rtype/Simulation.hpp"

#include <catch2/catch_test_macros.hpp>
#include <utility>

using rtype::game::GamePhase;
using rtype::game::GameRules;
using rtype::game::Simulation;

namespace {

constexpr std::uint32_t SEED = 42;

} // namespace

TEST_CASE("a new simulation starts at tick 0, waiting for players",
          "[game][simulation]")
{
    const Simulation simulation(GameRules{}, SEED);

    REQUIRE(simulation.tick() == 0);
    REQUIRE(simulation.phase() == GamePhase::WAITING);
}

TEST_CASE("the simulation keeps the rules it was built with",
          "[game][simulation]")
{
    GameRules rules;
    rules.tickRate = 30;
    rules.maxPlayers = 2;
    const Simulation simulation(rules, SEED);

    REQUIRE(simulation.rules().tickRate == 30);
    REQUIRE(simulation.rules().maxPlayers == 2);
}

TEST_CASE("step advances the tick by exactly one", "[game][simulation]")
{
    Simulation simulation(GameRules{}, SEED);

    simulation.step();
    REQUIRE(simulation.tick() == 1);
    for (int i = 0; i < 59; ++i) {
        simulation.step();
    }
    REQUIRE(simulation.tick() == 60);
}

TEST_CASE("an empty world has no events, states or spawns",
          "[game][simulation]")
{
    Simulation simulation(GameRules{}, SEED);

    simulation.step();
    REQUIRE(simulation.takeEvents().empty());
    REQUIRE(simulation.states().empty());
    REQUIRE(simulation.currentWorld().empty());
}

TEST_CASE("removing or driving an unknown player is safe",
          "[game][simulation]")
{
    Simulation simulation(GameRules{}, SEED);

    simulation.removePlayer(0);
    simulation.removePlayer(0);
    simulation.setInput(3, rtype::game::keys::UP | rtype::game::keys::SHOOT);
    simulation.step();
    REQUIRE(simulation.tick() == 1);
}

TEST_CASE("a simulation can be moved and keeps its state",
          "[game][simulation]")
{
    Simulation first(GameRules{}, SEED);

    first.step();
    const Simulation second(std::move(first));
    REQUIRE(second.tick() == 1);
}

TEST_CASE("the keys match the protocol bitmask", "[game][simulation]")
{
    using namespace rtype::game::keys;

    REQUIRE(UP == 0x01);
    REQUIRE(DOWN == 0x02);
    REQUIRE(LEFT == 0x04);
    REQUIRE(RIGHT == 0x08);
    REQUIRE(SHOOT == 0x10);
}
