/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Phases.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <utility>
#include <variant>
#include <vector>

#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/Components/Components.hpp"
#include "Rtype/GameContext.hpp"
#include "Rtype/Players.hpp"
#include "Rtype/Simulation.hpp"
#include "Rtype/Systems/GameStateSystem.hpp"
#include "Rtype/Systems/RespawnSystem.hpp"

using rtype::game::DestroyEvent;
using rtype::game::DestroyReason;
using rtype::game::EntityKind;
using rtype::game::EntityState;
using rtype::game::GameContext;
using rtype::game::GameEvent;
using rtype::game::GamePhase;
using rtype::game::GameRules;
using rtype::game::GameStateEvent;
using rtype::game::NO_PLAYER;
using rtype::game::ScoreEvent;
using rtype::game::Simulation;
using rtype::game::SpawnEvent;

namespace {

using Timed = std::pair<std::uint64_t, GameEvent>;

constexpr std::uint32_t SEED = 5;
constexpr int ONE_SECOND = 60;

/** A game where enemies come fast and a stationary player dies quickly. */
GameRules underFire()
{
    GameRules rules;

    rules.enemySpawnInterval = 0.2F;
    rules.enemyFireInterval = 0.3F;
    rules.gameOverDelay = 1.0F;
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

/** The phase changes of a log (a level-up keeps the phase and is skipped). */
std::vector<std::pair<std::uint64_t, GameStateEvent>>
statesOf(const std::vector<Timed> &log)
{
    std::vector<std::pair<std::uint64_t, GameStateEvent>> found;
    GamePhase last = GamePhase::WAITING;

    for (const auto &[tick, event] : log) {
        const auto *state = std::get_if<GameStateEvent>(&event);

        if (state != nullptr && state->phase != last) {
            found.emplace_back(tick, *state);
            last = state->phase;
        }
    }
    return found;
}

std::size_t countOfKind(const Simulation &simulation, EntityKind kind)
{
    std::size_t count = 0;

    for (const EntityState &state : simulation.states()) {
        count += (state.kind == kind) ? 1U : 0U;
    }
    return count;
}

std::size_t notShips(const Simulation &simulation)
{
    return simulation.states().size() -
           countOfKind(simulation, EntityKind::PLAYER_SHIP);
}

/** Steps until the phase changes to the one asked, at most limit ticks. */
bool stepUntil(Simulation &simulation, GamePhase phase, int limit)
{
    for (int i = 0; i < limit && simulation.phase() != phase; ++i) {
        simulation.step();
    }
    return simulation.phase() == phase;
}

/** The systems of the phases alone, to set the state by hand. */
struct Scene {
    explicit Scene(const GameRules &rules)
        : context{rules}
    {
        context.players.resize(rules.maxPlayers);
        rtype::game::registerComponents(registry);
        rtype::game::addGameStateSystem(scheduler, context);
        rtype::game::addRespawnSystem(scheduler, context);
    }

    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;
    Scene(Scene &&) = delete;
    Scene &operator=(Scene &&) = delete;
    ~Scene() = default;

    GameContext context;
    engine::ecs::Registry registry;
    engine::ecs::Scheduler scheduler;
};

} // namespace

TEST_CASE("the game waits while nobody is in it", "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.phase() == GamePhase::WAITING);
    REQUIRE(simulation.level() == 1);
    REQUIRE(run(simulation, 5 * ONE_SECOND).empty());
    REQUIRE(simulation.phase() == GamePhase::WAITING);
}

TEST_CASE("the first player starts the game on the next tick", "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    REQUIRE(simulation.phase() == GamePhase::WAITING);

    simulation.step();
    REQUIRE(simulation.phase() == GamePhase::PLAYING);
    REQUIRE(simulation.level() == 1);
}

TEST_CASE("starting the game is reported once, after the ship appears",
          "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    simulation.step();
    const auto events = simulation.takeEvents();

    REQUIRE(events.size() == 2);
    REQUIRE(std::holds_alternative<SpawnEvent>(events[0]));
    REQUIRE(std::holds_alternative<GameStateEvent>(events[1]));
    REQUIRE(std::get<GameStateEvent>(events[1]).phase == GamePhase::PLAYING);
    REQUIRE(std::get<GameStateEvent>(events[1]).level == 1);
    REQUIRE(statesOf(run(simulation, 1)).empty());
}

TEST_CASE("more players do not change the phase", "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    simulation.step();
    static_cast<void>(simulation.takeEvents());
    REQUIRE(simulation.addPlayer(1));
    REQUIRE(simulation.addPlayer(2));

    REQUIRE(statesOf(run(simulation, 1)).empty());
    REQUIRE(simulation.phase() == GamePhase::PLAYING);
}

TEST_CASE("the game goes back to waiting and is cleared when everybody leaves",
          "[game][phases]")
{
    GameRules rules = underFire();
    rules.lives = 255;
    rules.enemiesPerLevel = 3;
    Simulation simulation(rules, SEED);

    REQUIRE(simulation.addPlayer(0));
    static_cast<void>(run(simulation, 5 * ONE_SECOND));
    REQUIRE(simulation.level() > 1);
    const std::size_t leftovers = notShips(simulation);
    REQUIRE(leftovers > 0);

    simulation.removePlayer(0);
    static_cast<void>(simulation.takeEvents());
    simulation.step();
    const auto events = simulation.takeEvents();
    std::size_t destroys = 0;

    for (const GameEvent &event : events) {
        destroys += std::holds_alternative<DestroyEvent>(event) ? 1U : 0U;
    }
    REQUIRE(destroys == leftovers);
    REQUIRE(std::holds_alternative<GameStateEvent>(events.back()));
    REQUIRE(std::get<GameStateEvent>(events.back()).phase ==
            GamePhase::WAITING);
    REQUIRE(std::get<GameStateEvent>(events.back()).level == 1);
    REQUIRE(simulation.phase() == GamePhase::WAITING);
    REQUIRE(simulation.level() == 1);
    REQUIRE(simulation.states().empty());
}

TEST_CASE("the game goes on while one player is left", "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    REQUIRE(simulation.addPlayer(1));
    simulation.step();
    static_cast<void>(simulation.takeEvents());
    simulation.removePlayer(0);

    REQUIRE(statesOf(run(simulation, 1)).empty());
    REQUIRE(simulation.phase() == GamePhase::PLAYING);
}

TEST_CASE("a new game starts after the game over delay", "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    const auto log = run(simulation, 120 * ONE_SECOND);
    const auto phases = statesOf(log);

    REQUIRE(phases.size() >= 3);
    REQUIRE(phases[0].second.phase == GamePhase::PLAYING);
    REQUIRE(phases[1].second.phase == GamePhase::GAME_OVER);
    REQUIRE(phases[2].second.phase == GamePhase::PLAYING);
    REQUIRE(phases[2].second.level == 1);
    REQUIRE(phases[2].first - phases[1].first >= 60);
    REQUIRE(phases[2].first - phases[1].first <= 61);
}

TEST_CASE("phases always follow each other in the same cycle", "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    const auto phases = statesOf(run(simulation, 200 * ONE_SECOND));

    REQUIRE(phases.size() >= 5);
    for (std::size_t i = 0; i < phases.size(); ++i) {
        const GamePhase expected =
            (i % 2 == 0) ? GamePhase::PLAYING : GamePhase::GAME_OVER;

        REQUIRE(phases[i].second.phase == expected);
    }
}

TEST_CASE("the new game starts from scratch with the players still there",
          "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    REQUIRE(stepUntil(simulation, GamePhase::GAME_OVER, 120 * ONE_SECOND));
    REQUIRE(notShips(simulation) > 0);
    static_cast<void>(simulation.takeEvents());

    REQUIRE(stepUntil(simulation, GamePhase::PLAYING, 2 * ONE_SECOND));
    const auto events = simulation.takeEvents();

    REQUIRE(countOfKind(simulation, EntityKind::PLAYER_SHIP) == 1);
    REQUIRE(notShips(simulation) == 0);
    REQUIRE(simulation.level() == 1);
    REQUIRE(std::holds_alternative<GameStateEvent>(events.back()));
}

TEST_CASE("a player who joins during the game over plays the next game",
          "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    REQUIRE(stepUntil(simulation, GamePhase::GAME_OVER, 120 * ONE_SECOND));
    REQUIRE(simulation.addPlayer(1));
    static_cast<void>(simulation.takeEvents());

    bool enemyAppeared = false;
    for (const Timed &entry : run(simulation, 50)) {
        const auto *spawn = std::get_if<SpawnEvent>(&entry.second);

        enemyAppeared = enemyAppeared ||
                        (spawn != nullptr && spawn->kind == EntityKind::BYDOS);
    }
    REQUIRE_FALSE(enemyAppeared);

    REQUIRE(stepUntil(simulation, GamePhase::PLAYING, 2 * ONE_SECOND));
    REQUIRE(countOfKind(simulation, EntityKind::PLAYER_SHIP) == 2);
}

TEST_CASE("nobody left during the game over goes back to waiting",
          "[game][phases]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    REQUIRE(stepUntil(simulation, GamePhase::GAME_OVER, 120 * ONE_SECOND));
    simulation.removePlayer(0);
    simulation.step();

    REQUIRE(simulation.phase() == GamePhase::WAITING);
    REQUIRE(simulation.states().empty());
}

TEST_CASE("without a delay the game over lasts until everybody leaves",
          "[game][phases]")
{
    GameRules rules = underFire();
    rules.gameOverDelay = 0.0F;
    Simulation simulation(rules, SEED);

    REQUIRE(simulation.addPlayer(0));
    REQUIRE(stepUntil(simulation, GamePhase::GAME_OVER, 120 * ONE_SECOND));
    static_cast<void>(run(simulation, 60 * ONE_SECOND));
    REQUIRE(simulation.phase() == GamePhase::GAME_OVER);

    simulation.removePlayer(0);
    simulation.step();
    REQUIRE(simulation.phase() == GamePhase::WAITING);
}

TEST_CASE("a new game resets the scores, the lives and the waves",
          "[game][phases]")
{
    GameRules rules;
    rules.gameOverDelay = 1.0F;
    Scene scene(rules);
    rtype::game::PlayerState &player = scene.context.players[2];

    player.present = true;
    player.score = 300;
    player.lives = 0;
    scene.context.level = 4;
    scene.context.wave.enemiesSpawned = 35;
    rtype::game::checkGameOver(scene.context);
    REQUIRE(scene.context.phase == GamePhase::GAME_OVER);
    scene.context.events.clear();

    for (int i = 0; i < 70; ++i) {
        scene.scheduler.run(scene.registry, 1.0 / 60.0);
    }
    const auto events = std::exchange(scene.context.events, {});

    REQUIRE(events.size() == 3);
    REQUIRE(std::get<ScoreEvent>(events[0]).player == 2);
    REQUIRE(std::get<ScoreEvent>(events[0]).score == 0);
    REQUIRE(std::get<SpawnEvent>(events[1]).kind == EntityKind::PLAYER_SHIP);
    REQUIRE(std::get<SpawnEvent>(events[1]).owner == 2);
    REQUIRE(std::get<GameStateEvent>(events[2]).phase == GamePhase::PLAYING);
    REQUIRE(std::get<GameStateEvent>(events[2]).level == 1);
    REQUIRE(player.score == 0);
    REQUIRE(player.lives == rules.lives);
    REQUIRE(player.ship.has_value());
    REQUIRE(scene.context.level == 1);
    REQUIRE(scene.context.wave.enemiesSpawned == 0);
}

TEST_CASE("the world describes every living entity for a late joiner",
          "[game][world]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    static_cast<void>(run(simulation, 3 * ONE_SECOND));
    const auto world = simulation.currentWorld();
    const auto states = simulation.states();

    REQUIRE(world.size() == states.size());
    REQUIRE(states.size() > 1);
    for (std::size_t i = 0; i < world.size(); ++i) {
        REQUIRE(world[i].netId == states[i].netId);
        REQUIRE(world[i].kind == states[i].kind);
        REQUIRE(world[i].position == states[i].position);
    }
}

TEST_CASE("the world contains every kind of entity", "[game][world]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    simulation.setInput(0, rtype::game::keys::SHOOT);
    static_cast<void>(run(simulation, 3 * ONE_SECOND));

    REQUIRE(countOfKind(simulation, EntityKind::PLAYER_SHIP) +
                countOfKind(simulation, EntityKind::PLAYER_MISSILE) +
                countOfKind(simulation, EntityKind::BYDOS) +
                countOfKind(simulation, EntityKind::BYDOS_MISSILE) ==
            simulation.states().size());
    REQUIRE(countOfKind(simulation, EntityKind::BYDOS) > 0);
    REQUIRE(countOfKind(simulation, EntityKind::PLAYER_MISSILE) > 0);
}

TEST_CASE("events come in the order things happened", "[game][events]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    const auto log = run(simulation, 10 * ONE_SECOND);
    std::uint64_t last = 0;
    std::vector<std::uint16_t> alive;

    for (const auto &[tick, event] : log) {
        REQUIRE(tick >= last);
        last = tick;
        if (const auto *spawn = std::get_if<SpawnEvent>(&event)) {
            alive.push_back(spawn->netId);
        }
        if (const auto *destroy = std::get_if<DestroyEvent>(&event)) {
            bool known = false;
            for (const std::uint16_t id : alive) {
                known = known || id == destroy->netId;
            }
            REQUIRE(known);
        }
    }
}

TEST_CASE("the events are given once and then cleared", "[game][events]")
{
    Simulation simulation(underFire(), SEED);

    REQUIRE(simulation.addPlayer(0));
    simulation.step();

    REQUIRE_FALSE(simulation.takeEvents().empty());
    REQUIRE(simulation.takeEvents().empty());
}
