/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Lives.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <utility>
#include <variant>
#include <vector>

#include "Rtype/Simulation.hpp"

using rtype::game::DestroyEvent;
using rtype::game::DestroyReason;
using rtype::game::EntityKind;
using rtype::game::GameEvent;
using rtype::game::GamePhase;
using rtype::game::GameRules;
using rtype::game::GameStateEvent;
using rtype::game::NO_PLAYER;
using rtype::game::Simulation;
using rtype::game::SpawnEvent;

namespace {

using Timed = std::pair<std::uint64_t, GameEvent>;

constexpr std::uint8_t SLOT = 0;
constexpr std::uint32_t SEED = 5;

GameRules underFire()
{
    GameRules rules;

    rules.enemySpawnInterval = 0.2F;
    rules.enemyFireInterval = 0.3F;
    return rules;
}

std::vector<Timed> run(Simulation &simulation, int steps)
{
    std::vector<Timed> log;

    for (int i = 0; i < steps; ++i) {
        simulation.step();
        for (GameEvent &event : simulation.takeEvents()) {
            log.emplace_back(simulation.tick(), std::move(event));
        }
    }
    return log;
}

std::vector<Timed> opening(Simulation &simulation)
{
    std::vector<Timed> log;

    for (GameEvent &event : simulation.takeEvents()) {
        log.emplace_back(0, std::move(event));
    }
    return log;
}

void append(std::vector<Timed> &log, std::vector<Timed> more)
{
    for (Timed &entry : more) {
        log.push_back(std::move(entry));
    }
}

struct Life {
    std::uint64_t diedAt = 0;
    std::uint64_t backAt = 0;
};

/**
 * Follows the ships of a player: the ids of the ships spawned, the tick of
 * each death, and the tick of the next spawn.
 */
std::vector<Life> lives(const std::vector<Timed> &log)
{
    std::vector<std::uint16_t> shipIds;
    std::vector<Life> found;

    for (const auto &[tick, event] : log) {
        const auto *spawn = std::get_if<SpawnEvent>(&event);
        const auto *destroy = std::get_if<DestroyEvent>(&event);

        if (spawn != nullptr && spawn->kind == EntityKind::PLAYER_SHIP) {
            shipIds.push_back(spawn->netId);
            if (!found.empty() && found.back().backAt == 0) {
                found.back().backAt = tick;
            }
        }
        if (destroy != nullptr && !shipIds.empty() &&
            destroy->netId == shipIds.back()) {
            found.push_back({tick, 0});
        }
    }
    return found;
}

std::size_t gameOvers(const std::vector<Timed> &log)
{
    std::size_t count = 0;

    for (const auto &[tick, event] : log) {
        const auto *state = std::get_if<GameStateEvent>(&event);

        count += (state != nullptr && state->phase == GamePhase::GAME_OVER)
                     ? 1U
                     : 0U;
    }
    return count;
}

} // namespace

TEST_CASE("a player under fire loses all the lives and the game ends",
          "[game][lives]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(SLOT));
    auto log = opening(simulation);
    append(log, run(simulation, 120 * 60));
    const auto deaths = lives(log);

    REQUIRE(deaths.size() == 3);
    REQUIRE(gameOvers(log) == 1);
    REQUIRE(simulation.phase() == GamePhase::GAME_OVER);
}

TEST_CASE(
    "each death is followed by a respawn after the delay, except the last",
    "[game][lives]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(SLOT));
    auto log = opening(simulation);
    append(log, run(simulation, 120 * 60));
    const auto deaths = lives(log);

    REQUIRE(deaths.size() == 3);
    for (std::size_t i = 0; i < 2; ++i) {
        REQUIRE(deaths[i].backAt - deaths[i].diedAt >= 120);
        REQUIRE(deaths[i].backAt - deaths[i].diedAt <= 121);
    }
    REQUIRE(deaths[2].backAt == 0);
}

TEST_CASE("a ship killed by the enemies is reported as KILLED", "[game][lives]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(SLOT));
    const auto first = std::get<SpawnEvent>(simulation.takeEvents().front());
    const auto log = run(simulation, 120 * 60);
    bool found = false;

    for (const auto &[tick, event] : log) {
        const auto *destroy = std::get_if<DestroyEvent>(&event);

        if (destroy != nullptr && destroy->netId == first.netId) {
            found = true;
            REQUIRE(destroy->reason == DestroyReason::KILLED);
            REQUIRE(destroy->killer == NO_PLAYER);
        }
    }
    REQUIRE(found);
}

TEST_CASE("nothing happens after the game is over", "[game][lives]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(SLOT));
    static_cast<void>(simulation.takeEvents());
    static_cast<void>(run(simulation, 120 * 60));
    REQUIRE(simulation.phase() == GamePhase::GAME_OVER);

    REQUIRE(run(simulation, 10 * 60).empty());
    REQUIRE(simulation.states().size() <= 20);
}

TEST_CASE("a player who leaves while waiting to respawn does not come back",
          "[game][lives]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(SLOT));
    auto log = opening(simulation);
    while (lives(log).empty()) {
        REQUIRE(simulation.tick() < 60U * 60U);
        append(log, run(simulation, 1));
    }
    simulation.removePlayer(SLOT);
    static_cast<void>(simulation.takeEvents());

    const auto after = run(simulation, 10 * 60);

    for (const auto &[tick, event] : after) {
        const auto *spawn = std::get_if<SpawnEvent>(&event);

        REQUIRE((spawn == nullptr || spawn->kind != EntityKind::PLAYER_SHIP));
    }
    REQUIRE(simulation.phase() == GamePhase::WAITING);
}

TEST_CASE("the same seed gives the same lives and the same end",
          "[game][lives]")
{
    Simulation first(underFire(), 77);
    Simulation second(underFire(), 77);

    REQUIRE(first.addPlayer(SLOT));
    REQUIRE(second.addPlayer(SLOT));
    auto logA = opening(first);
    auto logB = opening(second);
    append(logA, run(first, 60 * 60));
    append(logB, run(second, 60 * 60));
    const auto a = lives(logA);
    const auto b = lives(logB);

    REQUIRE(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        REQUIRE(a[i].diedAt == b[i].diedAt);
        REQUIRE(a[i].backAt == b[i].backAt);
    }
}
