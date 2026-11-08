/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Simulation.cpp
*/

#include "Rtype/Simulation.hpp"

#include <algorithm>
#include <optional>
#include <utility>

#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/Components/Components.hpp"
#include "Rtype/Entities.hpp"
#include "Rtype/GameContext.hpp"
#include "Rtype/Players.hpp"
#include "Rtype/Systems/BoundsSystem.hpp"
#include "Rtype/Systems/CollisionSystem.hpp"
#include "Rtype/Systems/DamageSystem.hpp"
#include "Rtype/Systems/EnemyAISystem.hpp"
#include "Rtype/Systems/InputSystem.hpp"
#include "Rtype/Systems/MovementSystem.hpp"
#include "Rtype/Systems/RespawnSystem.hpp"
#include "Rtype/Systems/SpawnSystem.hpp"

namespace rtype::game {

struct Simulation::World {
    GameContext context;
    std::uint32_t seed = 0;
    double stepTime = 1.0;
    std::uint64_t tick = 0;
    engine::ecs::Registry registry{};
    engine::ecs::Scheduler scheduler{};
};

Simulation::Simulation(const GameRules &rules, std::uint32_t seed)
    : _world(std::make_unique<World>(
          World{GameContext{rules,
                            NetIdAllocator{},
                            {},
                            Random{seed},
                            WaveState{rules.enemySpawnInterval, 0}},
                seed, 1.0 / std::max<std::uint32_t>(1, rules.tickRate)}))
{
    World &world = *_world;

    world.context.players.resize(rules.maxPlayers);
    registerComponents(world.registry);
    addRespawnSystem(world.scheduler, world.context);
    addInputSystem(world.scheduler, world.registry, world.context);
    addSpawnSystem(world.scheduler, world.context);
    addEnemyAISystem(world.scheduler, world.registry, world.context);
    addMovementSystems(world.scheduler, world.context.rules);
    addCollisionSystem(world.scheduler, world.context);
    addDamageSystem(world.scheduler, world.context);
    addBoundsSystem(world.scheduler, world.context);
}

Simulation::~Simulation() = default;

Simulation::Simulation(Simulation &&) noexcept = default;

Simulation &Simulation::operator=(Simulation &&) noexcept = default;

bool Simulation::addPlayer(std::uint8_t player)
{
    GameContext &context = _world->context;

    if (player >= context.players.size() || context.players[player].present) {
        return false;
    }
    context.players[player] = PlayerState{};
    context.players[player].present = true;
    context.players[player].lives = context.rules.lives;
    context.players[player].ship = spawnShip(_world->registry, context, player);
    if (!context.players[player].ship.has_value()) {
        context.players[player] = PlayerState{};
        return false;
    }
    return true;
}

void Simulation::removePlayer(std::uint8_t player)
{
    GameContext &context = _world->context;

    if (player >= context.players.size() || !context.players[player].present) {
        return;
    }
    PlayerState &state = context.players[player];

    if (state.ship.has_value()) {
        destroyEntity(_world->registry, context, *state.ship,
                      DestroyReason::LEFT_SCREEN, NO_PLAYER);
    }
    state = PlayerState{};
    checkGameOver(context);
}

void Simulation::setInput(std::uint8_t player, std::uint8_t pressed)
{
    GameContext &context = _world->context;

    if (player >= context.players.size() || !context.players[player].present) {
        return;
    }
    PlayerState &state = context.players[player];

    state.keys = pressed & keys::MASK;
    if (!state.ship.has_value()) {
        return;
    }
    auto *control =
        _world->registry.getComponents<PlayerControl>().get(*state.ship);

    if (control != nullptr) {
        control->keys = state.keys;
    }
}

void Simulation::step()
{
    _world->scheduler.run(_world->registry, _world->stepTime);
    ++_world->tick;
}

std::vector<GameEvent> Simulation::takeEvents()
{
    return std::exchange(_world->context.events, {});
}

std::vector<EntityState> Simulation::states() const
{
    return collectStates(_world->registry);
}

std::vector<SpawnEvent> Simulation::currentWorld() const
{
    return collectSpawns(_world->registry);
}

std::uint64_t Simulation::tick() const noexcept
{
    return _world->tick;
}

GamePhase Simulation::phase() const noexcept
{
    return _world->context.phase;
}

const GameRules &Simulation::rules() const noexcept
{
    return _world->context.rules;
}

} /* namespace rtype::game */
