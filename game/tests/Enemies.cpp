/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Enemies.cpp
*/

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "Rtype/Entities.hpp"
#include "Rtype/Simulation.hpp"

using Catch::Approx;
using rtype::game::DestroyEvent;
using rtype::game::DestroyReason;
using rtype::game::ENEMY_HEIGHT;
using rtype::game::ENEMY_WIDTH;
using rtype::game::EnemyPattern;
using rtype::game::EntityKind;
using rtype::game::EntityState;
using rtype::game::GameEvent;
using rtype::game::GameRules;
using rtype::game::GameStateEvent;
using rtype::game::NO_PLAYER;
using rtype::game::Simulation;
using rtype::game::SpawnEvent;
namespace keys = rtype::game::keys;

namespace {

using Timed = std::pair<std::uint64_t, GameEvent>;

constexpr std::uint32_t SEED = 11;
constexpr std::uint8_t SLOT = 0;
constexpr int ONE_SECOND = 60;

Simulation withPlayer(const GameRules &rules, std::uint32_t seed)
{
    Simulation simulation(rules, seed);

    REQUIRE(simulation.addPlayer(SLOT));
    static_cast<void>(simulation.takeEvents());
    return simulation;
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

std::vector<std::pair<std::uint64_t, SpawnEvent>>
spawnsOf(const std::vector<Timed> &log, EntityKind kind)
{
    std::vector<std::pair<std::uint64_t, SpawnEvent>> found;

    for (const auto &[tick, event] : log) {
        const auto *spawn = std::get_if<SpawnEvent>(&event);

        if (spawn != nullptr && spawn->kind == kind) {
            found.emplace_back(tick, *spawn);
        }
    }
    return found;
}

std::optional<EntityState> find(const Simulation &simulation,
                                std::uint16_t netId)
{
    for (const EntityState &state : simulation.states()) {
        if (state.netId == netId) {
            return state;
        }
    }
    return std::nullopt;
}

std::string describe(const GameEvent &event)
{
    return std::visit(
        [](const auto &typed) {
            using Type = std::decay_t<decltype(typed)>;

            if constexpr (std::is_same_v<Type, SpawnEvent>) {
                return "spawn " + std::to_string(typed.netId) + " " +
                       std::to_string(static_cast<int>(typed.kind)) + " " +
                       std::to_string(typed.variant) + " " +
                       std::to_string(typed.position.x) + " " +
                       std::to_string(typed.position.y) + " " +
                       std::to_string(typed.velocity.x);
            } else if constexpr (std::is_same_v<Type, DestroyEvent>) {
                return "destroy " + std::to_string(typed.netId);
            } else if constexpr (std::is_same_v<Type, GameStateEvent>) {
                return "state " + std::to_string(typed.level);
            } else {
                return std::string("score");
            }
        },
        event);
}

std::vector<std::string> transcript(const std::vector<Timed> &log)
{
    std::vector<std::string> lines;

    for (const auto &[tick, event] : log) {
        lines.push_back(std::to_string(tick) + " " + describe(event));
    }
    return lines;
}

std::optional<SpawnEvent> firstEnemy(std::uint32_t seed)
{
    GameRules rules;
    rules.enemySpawnInterval = 0.1F;
    Simulation simulation = withPlayer(rules, seed);
    const auto enemies = spawnsOf(run(simulation, 30), EntityKind::BYDOS);

    if (enemies.empty()) {
        return std::nullopt;
    }
    return enemies.front().second;
}

std::optional<std::uint32_t> seedWith(EnemyPattern pattern)
{
    for (std::uint32_t seed = 0; seed < 50; ++seed) {
        const auto enemy = firstEnemy(seed);

        if (enemy && enemy->variant == static_cast<std::uint8_t>(pattern)) {
            return seed;
        }
    }
    return std::nullopt;
}

} // namespace

TEST_CASE("no enemy appears while nobody plays", "[game][enemies]")
{
    Simulation simulation(GameRules{}, SEED);

    REQUIRE(run(simulation, 10 * ONE_SECOND).empty());
    REQUIRE(simulation.states().empty());
}

TEST_CASE("enemies stop appearing when the last player leaves",
          "[game][enemies]")
{
    Simulation simulation = withPlayer(GameRules{}, SEED);

    REQUIRE_FALSE(
        spawnsOf(run(simulation, 5 * ONE_SECOND), EntityKind::BYDOS).empty());
    simulation.removePlayer(SLOT);
    static_cast<void>(simulation.takeEvents());
    REQUIRE(
        spawnsOf(run(simulation, 5 * ONE_SECOND), EntityKind::BYDOS).empty());
}

TEST_CASE("an enemy appears after the spawn interval, then regularly",
          "[game][enemies]")
{
    GameRules rules;
    rules.enemySpawnInterval = 1.0F;
    Simulation simulation = withPlayer(rules, SEED);
    const auto enemies =
        spawnsOf(run(simulation, 4 * ONE_SECOND), EntityKind::BYDOS);

    REQUIRE(enemies.size() >= 3);
    REQUIRE(enemies[0].first >= 60);
    REQUIRE(enemies[0].first <= 61);
    REQUIRE(enemies[1].first - enemies[0].first >= 59);
    REQUIRE(enemies[1].first - enemies[0].first <= 61);
}

TEST_CASE("an enemy appears just outside the right edge", "[game][enemies]")
{
    const auto enemy = firstEnemy(SEED);
    const GameRules rules;

    REQUIRE(enemy.has_value());
    REQUIRE(enemy->kind == EntityKind::BYDOS);
    REQUIRE(enemy->owner == NO_PLAYER);
    REQUIRE(enemy->position.x == rules.worldWidth + ENEMY_WIDTH / 2.0F);
    REQUIRE(enemy->position.y >= ENEMY_HEIGHT);
    REQUIRE(enemy->position.y <= rules.worldHeight - ENEMY_HEIGHT);
    REQUIRE(enemy->velocity.x == -rules.enemySpeed);
}

TEST_CASE("the seed decides the height and the pattern of the enemies",
          "[game][enemies]")
{
    GameRules rules;
    rules.enemySpawnInterval = 0.1F;
    Simulation first = withPlayer(rules, 1);
    Simulation second = withPlayer(rules, 2);
    const auto a = spawnsOf(run(first, 120), EntityKind::BYDOS);
    const auto b = spawnsOf(run(second, 120), EntityKind::BYDOS);
    bool different = false;

    REQUIRE(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        different =
            different || a[i].second.position.y != b[i].second.position.y;
    }
    REQUIRE(different);
}

TEST_CASE("both patterns happen", "[game][enemies]")
{
    REQUIRE(seedWith(EnemyPattern::STRAIGHT).has_value());
    REQUIRE(seedWith(EnemyPattern::SINE_WAVE).has_value());
}

TEST_CASE("a straight enemy flies left at constant speed", "[game][enemies]")
{
    GameRules rules;
    rules.enemySpawnInterval = 0.1F;
    Simulation simulation =
        withPlayer(rules, *seedWith(EnemyPattern::STRAIGHT));
    const auto enemy = spawnsOf(run(simulation, 30), EntityKind::BYDOS).front();
    const auto before = *find(simulation, enemy.second.netId);

    static_cast<void>(run(simulation, 20));
    const auto after = *find(simulation, enemy.second.netId);

    REQUIRE(after.position.y == before.position.y);
    REQUIRE(before.position.x - after.position.x ==
            Approx(20.0F * rules.enemySpeed / ONE_SECOND).margin(0.01));
}

TEST_CASE("a sine wave enemy swings up and down while flying left",
          "[game][enemies]")
{
    GameRules rules;
    rules.enemySpawnInterval = 0.1F;
    Simulation simulation =
        withPlayer(rules, *seedWith(EnemyPattern::SINE_WAVE));
    const auto enemy = spawnsOf(run(simulation, 12), EntityKind::BYDOS).front();
    const float start = enemy.second.position.y;
    float lowest = start;
    float highest = start;

    for (int i = 0; i < 2 * ONE_SECOND; ++i) {
        simulation.step();
        const auto state = find(simulation, enemy.second.netId);
        REQUIRE(state.has_value());
        lowest = std::min(lowest, state->position.y);
        highest = std::max(highest, state->position.y);
    }
    REQUIRE(highest - lowest > 30.0F);
    REQUIRE(highest - start <= 61.0F);
    REQUIRE(start - lowest <= 61.0F);
}

TEST_CASE("an enemy on the screen shoots to the left", "[game][enemies]")
{
    GameRules rules;
    rules.enemyFireInterval = 0.5F;
    Simulation simulation = withPlayer(rules, SEED);
    const auto log = run(simulation, 6 * ONE_SECOND);
    const auto shots = spawnsOf(log, EntityKind::BYDOS_MISSILE);
    const auto enemies = spawnsOf(log, EntityKind::BYDOS);

    REQUIRE_FALSE(enemies.empty());
    REQUIRE_FALSE(shots.empty());
    REQUIRE(shots[0].second.owner == NO_PLAYER);
    REQUIRE(shots[0].second.velocity.x == -rules.enemyMissileSpeed);
    REQUIRE(shots[0].second.velocity.y == 0.0F);
    REQUIRE(shots[0].first > enemies[0].first);
}

TEST_CASE("an enemy does not shoot before it is on the screen",
          "[game][enemies]")
{
    GameRules rules;
    rules.enemyFireInterval = 0.01F;
    Simulation simulation = withPlayer(rules, SEED);
    const auto log = run(simulation, 3 * ONE_SECOND);
    const auto enemies = spawnsOf(log, EntityKind::BYDOS);

    REQUIRE_FALSE(enemies.empty());
    for (const auto &[tick, shot] : spawnsOf(log, EntityKind::BYDOS_MISSILE)) {
        REQUIRE(shot.position.x < rules.worldWidth);
    }
}

TEST_CASE("an enemy that leaves the screen is destroyed", "[game][enemies]")
{
    GameRules rules;
    rules.enemySpeed = 1000.0F;
    Simulation simulation = withPlayer(rules, SEED);
    const auto log = run(simulation, 5 * ONE_SECOND);
    const auto id = spawnsOf(log, EntityKind::BYDOS).front().second.netId;
    bool destroyed = false;

    for (const auto &[tick, event] : log) {
        const auto *destroy = std::get_if<DestroyEvent>(&event);

        if (destroy != nullptr && destroy->netId == id) {
            destroyed = true;
            REQUIRE(destroy->reason == DestroyReason::LEFT_SCREEN);
            REQUIRE(destroy->killer == NO_PLAYER);
        }
    }
    REQUIRE(destroyed);
    REQUIRE_FALSE(find(simulation, id).has_value());
}

TEST_CASE("an enemy missile that leaves the screen is destroyed",
          "[game][enemies]")
{
    GameRules rules;
    rules.enemyFireInterval = 0.5F;
    rules.enemyMissileSpeed = 2000.0F;
    Simulation simulation = withPlayer(rules, SEED);
    const auto log = run(simulation, 6 * ONE_SECOND);
    const auto id =
        spawnsOf(log, EntityKind::BYDOS_MISSILE).front().second.netId;
    bool destroyed = false;

    for (const auto &[tick, event] : log) {
        const auto *destroy = std::get_if<DestroyEvent>(&event);

        destroyed = destroyed || (destroy != nullptr && destroy->netId == id);
    }
    REQUIRE(destroyed);
}

TEST_CASE("the level goes up with the enemies and speeds them up",
          "[game][enemies]")
{
    GameRules rules;
    rules.enemySpawnInterval = 0.1F;
    rules.enemiesPerLevel = 3;
    Simulation simulation = withPlayer(rules, SEED);
    const auto log = run(simulation, 3 * ONE_SECOND);
    std::size_t enemies = 0;
    std::vector<std::uint8_t> levels;

    for (const auto &[tick, event] : log) {
        const auto *spawn = std::get_if<SpawnEvent>(&event);
        const auto *state = std::get_if<GameStateEvent>(&event);

        if (spawn != nullptr && spawn->kind == EntityKind::BYDOS) {
            const auto level = static_cast<std::uint8_t>(1 + enemies / 3);
            REQUIRE(spawn->velocity.x ==
                    Approx(-rtype::game::enemySpeed(rules, level)));
            ++enemies;
        }
        if (state != nullptr && state->level > 1) {
            levels.push_back(state->level);
            REQUIRE(enemies == 3 * levels.size());
            REQUIRE(state->phase == simulation.phase());
        }
    }
    REQUIRE(levels.size() >= 3);
    REQUIRE(levels[0] == 2);
    REQUIRE(levels[1] == 3);
    REQUIRE(levels[2] == 4);
}

TEST_CASE("the same seed and inputs give exactly the same events",
          "[game][enemies]")
{
    GameRules rules;
    rules.enemySpawnInterval = 0.3F;
    rules.enemiesPerLevel = 4;
    Simulation first = withPlayer(rules, 2024);
    Simulation second = withPlayer(rules, 2024);

    for (int i = 0; i < 20; ++i) {
        const std::uint8_t pressed =
            (i % 2 == 0) ? keys::UP | keys::SHOOT : keys::DOWN;
        first.setInput(SLOT, pressed);
        second.setInput(SLOT, pressed);
        REQUIRE(transcript(run(first, 30)) == transcript(run(second, 30)));
    }
}

TEST_CASE("another seed gives another game", "[game][enemies]")
{
    GameRules rules;
    rules.enemySpawnInterval = 0.3F;
    Simulation first = withPlayer(rules, 1);
    Simulation second = withPlayer(rules, 2);

    REQUIRE(transcript(run(first, 600)) != transcript(run(second, 600)));
}
