/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Entities.cpp
*/

#include "Rtype/Entities.hpp"

#include <tuple>

#include "Engine/Ecs/IndexedZipper.hpp"

namespace rtype::game {

namespace {

engine::core::Vec2 shipSpawnPosition(const GameRules &rules,
                                     std::uint8_t slot) noexcept
{
    const float rows = static_cast<float>(rules.maxPlayers) + 1.0F;
    const float row = static_cast<float>(slot) + 1.0F;

    return {rules.worldWidth * 0.1F, rules.worldHeight * row / rows};
}

void attachShip(engine::ecs::Registry &registry, engine::ecs::Entity entity,
                const SpawnEvent &spawn, std::uint8_t lives)
{
    registry.addComponent(entity, Position{spawn.position});
    registry.addComponent(entity, Velocity{});
    registry.addComponent(entity, Hitbox{SHIP_WIDTH, SHIP_HEIGHT});
    registry.addComponent(entity, NetId{spawn.netId});
    registry.addComponent(entity, Kind{spawn.kind, spawn.variant, spawn.owner});
    registry.addComponent(entity, PlayerControl{spawn.owner, 0, 0.0F});
    registry.addComponent(entity, Health{});
    registry.addComponent(entity, Lives{lives});
    registry.addComponent(entity, Team{TeamSide::PLAYERS});
}

void attachMissile(engine::ecs::Registry &registry, engine::ecs::Entity entity,
                   const SpawnEvent &spawn)
{
    registry.addComponent(entity, Position{spawn.position});
    registry.addComponent(entity, Velocity{spawn.velocity});
    registry.addComponent(entity, Hitbox{MISSILE_WIDTH, MISSILE_HEIGHT});
    registry.addComponent(entity, NetId{spawn.netId});
    registry.addComponent(entity, Kind{spawn.kind, spawn.variant, spawn.owner});
    registry.addComponent(entity, Team{TeamSide::PLAYERS});
}

} // namespace

std::optional<engine::ecs::Entity> spawnShip(engine::ecs::Registry &registry,
                                             GameContext &context,
                                             std::uint8_t slot)
{
    const auto netId = context.netIds.allocate();

    if (!netId) {
        return std::nullopt;
    }
    const SpawnEvent spawn{*netId,
                           EntityKind::PLAYER_SHIP,
                           0,
                           slot,
                           shipSpawnPosition(context.rules, slot),
                           {}};
    const engine::ecs::Entity entity = registry.spawnEntity();

    attachShip(registry, entity, spawn, context.rules.lives);
    context.events.emplace_back(spawn);
    return entity;
}

bool spawnMissile(engine::ecs::Registry &registry,
                  engine::ecs::CommandBuffer &commands, GameContext &context,
                  const Position &shipPosition, std::uint8_t owner)
{
    const auto netId = context.netIds.allocate();

    if (!netId) {
        return false;
    }
    const float reach = (SHIP_WIDTH + MISSILE_WIDTH) / 2.0F;
    const SpawnEvent spawn{*netId,
                           EntityKind::PLAYER_MISSILE,
                           0,
                           owner,
                           shipPosition.value + engine::core::Vec2{reach, 0.0F},
                           {context.rules.missileSpeed, 0.0F}};

    context.events.emplace_back(spawn);
    commands.spawn([&registry, spawn](engine::ecs::Entity entity) {
        attachMissile(registry, entity, spawn);
    });
    return true;
}

void destroyEntity(engine::ecs::Registry &registry, GameContext &context,
                   engine::ecs::Entity entity, DestroyReason reason,
                   std::uint8_t killer)
{
    const NetId *netId = registry.getComponents<NetId>().get(entity);

    if (netId != nullptr) {
        context.netIds.release(netId->value);
        context.events.emplace_back(DestroyEvent{netId->value, reason, killer});
    }
    registry.killEntity(entity);
}

std::vector<EntityState> collectStates(const engine::ecs::Registry &registry)
{
    std::vector<EntityState> states;

    for (auto row : engine::ecs::makeIndexedZipper(
             registry.getComponents<NetId>(), registry.getComponents<Kind>(),
             registry.getComponents<Position>())) {
        const auto &[index, netId, kind, position] = row;
        states.push_back({netId.value, kind.kind, position.value});
    }
    return states;
}

std::vector<SpawnEvent> collectSpawns(const engine::ecs::Registry &registry)
{
    std::vector<SpawnEvent> spawns;

    for (auto row : engine::ecs::makeIndexedZipper(
             registry.getComponents<NetId>(), registry.getComponents<Kind>(),
             registry.getComponents<Position>(),
             registry.getComponents<Velocity>())) {
        const auto &[index, netId, kind, position, velocity] = row;
        spawns.push_back({netId.value, kind.kind, kind.variant, kind.owner,
                          position.value, velocity.value});
    }
    return spawns;
}

} /* namespace rtype::game */
