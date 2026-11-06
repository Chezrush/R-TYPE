/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SpawnSystem.cpp
*/

#include "Rtype/Systems/SpawnSystem.hpp"

#include "Rtype/Entities.hpp"

namespace rtype::game {

namespace {

void advanceLevel(GameContext &context)
{
    const std::uint8_t level =
        levelAfter(context.wave.enemiesSpawned, context.rules);

    if (level > context.level) {
        context.level = level;
        context.events.emplace_back(GameStateEvent{context.phase, level});
    }
}

void updateWaves(engine::ecs::Registry &registry,
                 engine::ecs::CommandBuffer &commands, GameContext &context,
                 float deltaTime)
{
    WaveState &wave = context.wave;

    if (!hasPlayers(registry)) {
        return;
    }
    wave.spawnTimer -= deltaTime;
    if (wave.spawnTimer > 0.0F || !spawnEnemy(registry, commands, context)) {
        return;
    }
    wave.spawnTimer += context.rules.enemySpawnInterval;
    ++wave.enemiesSpawned;
    advanceLevel(context);
}

} // namespace

void addSpawnSystem(engine::ecs::Scheduler &scheduler, GameContext &context)
{
    scheduler.addSystem(
        "spawn", engine::ecs::Phase::PRE_UPDATE, engine::ecs::Access{},
        [&context](engine::ecs::Registry &registry,
                   engine::ecs::CommandBuffer &commands, double deltaTime) {
            updateWaves(registry, commands, context,
                        static_cast<float>(deltaTime));
        });
}

} /* namespace rtype::game */
