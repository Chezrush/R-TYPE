/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Combat.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/Components/Components.hpp"
#include "Rtype/Entities.hpp"
#include "Rtype/GameContext.hpp"
#include "Rtype/Systems/CollisionSystem.hpp"
#include "Rtype/Systems/DamageSystem.hpp"
#include "Rtype/Systems/RespawnSystem.hpp"

using engine::core::Vec2;
using engine::ecs::Entity;
using rtype::game::DestroyEvent;
using rtype::game::DestroyReason;
using rtype::game::EntityKind;
using rtype::game::GameContext;
using rtype::game::GameEvent;
using rtype::game::GamePhase;
using rtype::game::GameRules;
using rtype::game::GameStateEvent;
using rtype::game::NO_PLAYER;
using rtype::game::ScoreEvent;
using rtype::game::SpawnEvent;
using rtype::game::TeamSide;
namespace keys = rtype::game::keys;

namespace {

constexpr float STEP = 1.0F / 60.0F;
const Vec2 FAR_AWAY{900.0F, 600.0F};

/**
 * The systems of the combat, alone in a small world where the test places
 * every entity by hand.
 */
struct Scene {
    explicit Scene(const GameRules &rules = {})
        : context{rules}
    {
        context.players.resize(rules.maxPlayers);
        rtype::game::registerComponents(registry);
        rtype::game::addRespawnSystem(scheduler, context);
        rtype::game::addCollisionSystem(scheduler, context);
        rtype::game::addDamageSystem(scheduler, context);
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

void step(Scene &scene, int count = 1)
{
    for (int i = 0; i < count; ++i) {
        scene.scheduler.run(scene.registry, STEP);
    }
}

std::vector<GameEvent> drain(Scene &scene)
{
    return std::exchange(scene.context.events, {});
}

Entity addPlayer(Scene &scene, std::uint8_t slot, const Vec2 &position)
{
    rtype::game::PlayerState &player = scene.context.players.at(slot);

    player.present = true;
    player.lives = scene.context.rules.lives;
    player.ship = rtype::game::spawnShip(scene.registry, scene.context, slot);
    scene.registry.getComponents<rtype::game::Position>()
        .get(*player.ship)
        ->value = position;
    static_cast<void>(drain(scene));
    return *player.ship;
}

Entity place(Scene &scene, EntityKind kind, std::uint8_t owner,
             const Vec2 &position, int health = 1)
{
    const auto netId = scene.context.netIds.allocate();
    const Entity entity = scene.registry.spawnEntity();
    const bool players = kind == EntityKind::PLAYER_MISSILE;
    const TeamSide side = players ? TeamSide::PLAYERS : TeamSide::ENEMIES;

    scene.registry.addComponent(entity, rtype::game::Position{position});
    scene.registry.addComponent(entity, rtype::game::Hitbox{40.0F, 40.0F});
    scene.registry.addComponent(entity, rtype::game::NetId{*netId});
    scene.registry.addComponent(entity, rtype::game::Kind{kind, 0, owner});
    scene.registry.addComponent(entity, rtype::game::Health{health, health});
    scene.registry.addComponent(entity, rtype::game::Team{side});
    return entity;
}

Entity enemy(Scene &scene, const Vec2 &position, int health = 1)
{
    return place(scene, EntityKind::BYDOS, NO_PLAYER, position, health);
}

Entity missile(Scene &scene, std::uint8_t owner, const Vec2 &position)
{
    return place(scene, EntityKind::PLAYER_MISSILE, owner, position);
}

template <typename Event>
std::vector<Event> only(const std::vector<GameEvent> &events)
{
    std::vector<Event> found;

    for (const GameEvent &event : events) {
        if (const auto *typed = std::get_if<Event>(&event)) {
            found.push_back(*typed);
        }
    }
    return found;
}

std::optional<DestroyEvent> destroyOf(const std::vector<GameEvent> &events,
                                      std::uint16_t netId)
{
    for (const DestroyEvent &destroy : only<DestroyEvent>(events)) {
        if (destroy.netId == netId) {
            return destroy;
        }
    }
    return std::nullopt;
}

std::uint16_t netIdOf(const Scene &scene, Entity entity)
{
    return scene.registry.getComponents<rtype::game::NetId>()
        .get(entity)
        ->value;
}

} // namespace

TEST_CASE("a missile kills an enemy and its owner scores", "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 2, FAR_AWAY);
    const Entity target = enemy(scene, {500.0F, 300.0F});
    const Entity shot = missile(scene, 2, {500.0F, 300.0F});
    const std::uint16_t targetId = netIdOf(scene, target);
    const std::uint16_t shotId = netIdOf(scene, shot);

    step(scene);
    const auto events = drain(scene);
    const auto scores = only<ScoreEvent>(events);

    REQUIRE(destroyOf(events, targetId).has_value());
    REQUIRE(destroyOf(events, targetId)->reason == DestroyReason::KILLED);
    REQUIRE(destroyOf(events, targetId)->killer == 2);
    REQUIRE(destroyOf(events, shotId).has_value());
    REQUIRE(destroyOf(events, shotId)->killer == NO_PLAYER);
    REQUIRE(scores.size() == 1);
    REQUIRE(scores[0].player == 2);
    REQUIRE(scores[0].score == scene.context.rules.enemyScore);
    REQUIRE_FALSE(scene.registry.isAlive(target));
    REQUIRE_FALSE(scene.registry.isAlive(shot));
}

TEST_CASE("the score of a player adds up", "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 1, FAR_AWAY);

    enemy(scene, {500.0F, 300.0F});
    missile(scene, 1, {500.0F, 300.0F});
    step(scene);
    enemy(scene, {500.0F, 100.0F});
    missile(scene, 1, {500.0F, 100.0F});
    step(scene);

    const auto scores = only<ScoreEvent>(drain(scene));

    REQUIRE(scores.size() == 2);
    REQUIRE(scores[0].score == 100);
    REQUIRE(scores[1].score == 200);
    REQUIRE(scene.context.players[1].score == 200);
}

TEST_CASE("the same team never hits itself", "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 0, {500.0F, 300.0F});

    missile(scene, 0, {500.0F, 300.0F});
    enemy(scene, {100.0F, 100.0F});
    enemy(scene, {100.0F, 100.0F});
    place(scene, EntityKind::BYDOS_MISSILE, NO_PLAYER, {100.0F, 100.0F});
    step(scene, 5);

    REQUIRE(drain(scene).empty());
    REQUIRE(scene.context.players[0].lives == 3);
}

TEST_CASE("boxes that only touch do not hit", "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 0, FAR_AWAY);

    enemy(scene, {500.0F, 300.0F});
    missile(scene, 0, {540.0F, 300.0F});
    step(scene);

    REQUIRE(drain(scene).empty());
}

TEST_CASE("an entity with several health points survives a hit",
          "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 0, FAR_AWAY);
    const Entity boss = enemy(scene, {500.0F, 300.0F}, 3);
    const std::uint16_t bossId = netIdOf(scene, boss);

    for (int hit = 1; hit <= 2; ++hit) {
        missile(scene, 0, {500.0F, 300.0F});
        step(scene);
        REQUIRE_FALSE(destroyOf(drain(scene), bossId).has_value());
        REQUIRE(scene.registry.getComponents<rtype::game::Health>()
                    .get(boss)
                    ->current == 3 - hit);
    }
    missile(scene, 0, {500.0F, 300.0F});
    step(scene);
    REQUIRE(destroyOf(drain(scene), bossId)->killer == 0);
}

TEST_CASE("a missile that hits two enemies at once only kills one",
          "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 0, FAR_AWAY);
    const Entity first = enemy(scene, {500.0F, 300.0F});
    const Entity second = enemy(scene, {500.0F, 300.0F});

    missile(scene, 0, {500.0F, 300.0F});
    step(scene);

    REQUIRE(only<ScoreEvent>(drain(scene)).size() == 1);
    REQUIRE(scene.registry.isAlive(first) != scene.registry.isAlive(second));
}

TEST_CASE("an enemy hitting a ship destroys both and costs a life",
          "[game][combat]")
{
    Scene scene;
    const Entity ship = addPlayer(scene, 1, {500.0F, 300.0F});
    const Entity rammer = enemy(scene, {510.0F, 300.0F});
    const std::uint16_t shipId = netIdOf(scene, ship);
    const std::uint16_t rammerId = netIdOf(scene, rammer);

    step(scene);
    const auto events = drain(scene);

    REQUIRE(destroyOf(events, shipId)->killer == NO_PLAYER);
    REQUIRE(destroyOf(events, shipId)->reason == DestroyReason::KILLED);
    REQUIRE(destroyOf(events, rammerId)->killer == 1);
    REQUIRE(scene.context.players[1].lives == 2);
    REQUIRE_FALSE(scene.context.players[1].ship.has_value());
    REQUIRE_FALSE(scene.registry.isAlive(ship));
}

TEST_CASE("a dead player is back after the respawn delay", "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 1, {500.0F, 300.0F});
    enemy(scene, {510.0F, 300.0F});
    step(scene);
    static_cast<void>(drain(scene));

    int waited = 0;
    std::vector<SpawnEvent> spawns;

    while (spawns.empty() && waited < 200) {
        step(scene);
        ++waited;
        spawns = only<SpawnEvent>(drain(scene));
    }
    REQUIRE(waited >= 120);
    REQUIRE(waited <= 121);
    REQUIRE(spawns.size() == 1);
    REQUIRE(spawns[0].kind == EntityKind::PLAYER_SHIP);
    REQUIRE(spawns[0].owner == 1);
    REQUIRE(scene.context.players[1].ship.has_value());
    REQUIRE(scene.context.players[1].lives == 2);
}

TEST_CASE("the player is not back before the delay", "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 1, {500.0F, 300.0F});
    enemy(scene, {510.0F, 300.0F});
    step(scene, 1 + 118);

    REQUIRE_FALSE(scene.context.players[1].ship.has_value());
}

TEST_CASE("the new ship holds the keys the player was holding",
          "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 0, {500.0F, 300.0F});
    enemy(scene, {510.0F, 300.0F});
    step(scene);
    scene.context.players[0].keys = keys::RIGHT | keys::SHOOT;
    step(scene, 130);

    const auto &ship = scene.context.players[0].ship;

    REQUIRE(ship.has_value());
    REQUIRE(scene.registry.getComponents<rtype::game::PlayerControl>()
                .get(*ship)
                ->keys == (keys::RIGHT | keys::SHOOT));
}

TEST_CASE("losing the last life ends the game", "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 0, {500.0F, 300.0F});
    scene.context.players[0].lives = 1;
    enemy(scene, {510.0F, 300.0F});

    step(scene, 300);
    const auto states = only<GameStateEvent>(drain(scene));

    REQUIRE(scene.context.players[0].lives == 0);
    REQUIRE_FALSE(scene.context.players[0].ship.has_value());
    REQUIRE(only<SpawnEvent>(drain(scene)).empty());
    REQUIRE(scene.context.phase == GamePhase::GAME_OVER);
    REQUIRE(states.size() == 1);
    REQUIRE(states[0].phase == GamePhase::GAME_OVER);
}

TEST_CASE("the game goes on while another player has a ship or a life",
          "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 0, {500.0F, 300.0F});
    addPlayer(scene, 1, FAR_AWAY);
    scene.context.players[0].lives = 1;
    enemy(scene, {510.0F, 300.0F});

    step(scene);
    REQUIRE(scene.context.phase != GamePhase::GAME_OVER);

    scene.context.players[1].lives = 1;
    enemy(scene, {FAR_AWAY.x + 10.0F, FAR_AWAY.y});
    step(scene);
    REQUIRE(scene.context.phase == GamePhase::GAME_OVER);
    REQUIRE(only<GameStateEvent>(drain(scene)).size() == 1);
}

TEST_CASE("a ship hit twice in the same tick is destroyed once",
          "[game][combat]")
{
    Scene scene;
    const Entity ship = addPlayer(scene, 0, {500.0F, 300.0F});
    const std::uint16_t shipId = netIdOf(scene, ship);
    const Entity first = enemy(scene, {510.0F, 300.0F});
    const Entity second = enemy(scene, {490.0F, 300.0F});

    step(scene);
    const auto destroys = only<DestroyEvent>(drain(scene));
    std::size_t shipDestroys = 0;

    for (const DestroyEvent &destroy : destroys) {
        shipDestroys += (destroy.netId == shipId) ? 1U : 0U;
    }
    REQUIRE(shipDestroys == 1);
    REQUIRE(scene.context.players[0].lives == 2);
    REQUIRE_FALSE(scene.registry.isAlive(ship));
    REQUIRE(scene.registry.isAlive(first) != scene.registry.isAlive(second));
}

TEST_CASE("removing a waiting player cancels the respawn", "[game][combat]")
{
    Scene scene;
    addPlayer(scene, 0, {500.0F, 300.0F});
    enemy(scene, {510.0F, 300.0F});
    step(scene);
    scene.context.players[0] = rtype::game::PlayerState{};
    static_cast<void>(drain(scene));

    step(scene, 300);

    REQUIRE(drain(scene).empty());
    REQUIRE(scene.context.phase == GamePhase::WAITING);
}
