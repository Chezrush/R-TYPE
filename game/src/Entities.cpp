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
                   const SpawnEvent &spawn, TeamSide side)
{
    registry.addComponent(entity, Position{spawn.position});
    registry.addComponent(entity, Velocity{spawn.velocity});
    registry.addComponent(entity, Hitbox{MISSILE_WIDTH, MISSILE_HEIGHT});
    registry.addComponent(entity, NetId{spawn.netId});
    registry.addComponent(entity, Kind{spawn.kind, spawn.variant, spawn.owner});
    registry.addComponent(entity, Team{side});
}

void attachEnemy(engine::ecs::Registry &registry, engine::ecs::Entity entity,
                 const SpawnEvent &spawn, float fireInterval)
{
    const auto pattern = static_cast<EnemyPattern>(spawn.variant);

    registry.addComponent(entity, Position{spawn.position});
    registry.addComponent(entity, Velocity{spawn.velocity});
    registry.addComponent(entity, Hitbox{ENEMY_WIDTH, ENEMY_HEIGHT});
    registry.addComponent(entity, NetId{spawn.netId});
    registry.addComponent(entity, Kind{spawn.kind, spawn.variant, spawn.owner});
    registry.addComponent(entity, Health{});
    registry.addComponent(entity, Team{TeamSide::ENEMIES});
    registry.addComponent(entity, EnemyAI{pattern, 0.0F, fireInterval});
}

SpawnEvent makeEnemySpawn(GameContext &context, std::uint16_t netId)
{
    const GameRules &rules = context.rules;
    const float height = context.random.nextFloat(
        ENEMY_HEIGHT, rules.worldHeight - ENEMY_HEIGHT);
    const auto pattern = context.random.nextBool() ? EnemyPattern::SINE_WAVE
                                                   : EnemyPattern::STRAIGHT;

    return {netId,
            EntityKind::BYDOS,
            static_cast<std::uint8_t>(pattern),
            NO_PLAYER,
            {rules.worldWidth + ENEMY_WIDTH / 2.0F, height},
            {-enemySpeed(rules, context.level), 0.0F}};
}

void recordMissile(engine::ecs::Registry &registry,
                   engine::ecs::CommandBuffer &commands, GameContext &context,
                   const SpawnEvent &spawn, TeamSide side)
{
    context.events.emplace_back(spawn);
    commands.spawn([&registry, spawn, side](engine::ecs::Entity entity) {
        attachMissile(registry, entity, spawn, side);
    });
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

    recordMissile(registry, commands, context, spawn, TeamSide::PLAYERS);
    return true;
}

bool spawnEnemy(engine::ecs::Registry &registry,
                engine::ecs::CommandBuffer &commands, GameContext &context)
{
    const auto netId = context.netIds.allocate();

    if (!netId) {
        return false;
    }
    const SpawnEvent spawn = makeEnemySpawn(context, *netId);
    const float fireInterval = context.rules.enemyFireInterval;

    context.events.emplace_back(spawn);
    commands.spawn(
        [&registry, spawn, fireInterval](engine::ecs::Entity entity) {
            attachEnemy(registry, entity, spawn, fireInterval);
        });
    return true;
}

bool spawnEnemyMissile(engine::ecs::Registry &registry,
                       engine::ecs::CommandBuffer &commands,
                       GameContext &context, const Position &enemyPosition)
{
    const auto netId = context.netIds.allocate();

    if (!netId) {
        return false;
    }
    const float reach = (ENEMY_WIDTH + MISSILE_WIDTH) / 2.0F;
    const SpawnEvent spawn{*netId,
                           EntityKind::BYDOS_MISSILE,
                           0,
                           NO_PLAYER,
                           enemyPosition.value -
                               engine::core::Vec2{reach, 0.0F},
                           {-context.rules.enemyMissileSpeed, 0.0F}};

    recordMissile(registry, commands, context, spawn, TeamSide::ENEMIES);
    return true;
}

bool hasPlayers(const engine::ecs::Registry &registry)
{
    const auto players =
        engine::ecs::makeIndexedZipper(registry.getComponents<PlayerControl>());

    return players.begin() != players.end();
}

void announceDestroy(GameContext &context, std::uint16_t netId,
                     DestroyReason reason, std::uint8_t killer)
{
    context.netIds.release(netId);
    context.events.emplace_back(DestroyEvent{netId, reason, killer});
}

void destroyEntity(engine::ecs::Registry &registry, GameContext &context,
                   engine::ecs::Entity entity, DestroyReason reason,
                   std::uint8_t killer)
{
    const NetId *netId = registry.getComponents<NetId>().get(entity);

    if (netId != nullptr) {
        announceDestroy(context, netId->value, reason, killer);
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
