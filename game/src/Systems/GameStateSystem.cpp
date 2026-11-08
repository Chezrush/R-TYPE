/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameStateSystem.cpp
*/

#include "Rtype/Systems/GameStateSystem.hpp"

#include <utility>

#include "Engine/Ecs/IndexedZipper.hpp"
#include "Rtype/Components/Components.hpp"
#include "Rtype/Entities.hpp"
#include "Rtype/Phases.hpp"
#include "Rtype/Players.hpp"

namespace rtype::game {

namespace {

void clearEntities(engine::ecs::Registry &registry,
                   engine::ecs::CommandBuffer &commands, GameContext &context)
{
    const engine::ecs::Registry &view = registry;

    for (auto row : engine::ecs::makeIndexedZipper(
             view.getComponents<NetId>(), view.getComponents<Kind>())) {
        const auto &[index, netId, kind] = row;

        if (kind.kind != EntityKind::PLAYER_SHIP) {
            announceDestroy(context, netId.value, DestroyReason::LEFT_SCREEN,
                            NO_PLAYER);
            commands.kill(registry.entityAt(index));
        }
    }
}

void resetPlayer(engine::ecs::Registry &registry, GameContext &context,
                 std::uint8_t slot)
{
    PlayerState &player = context.players[slot];

    player.lives = context.rules.lives;
    player.respawnTimer = 0.0F;
    if (player.score != 0) {
        player.score = 0;
        context.events.emplace_back(ScoreEvent{slot, 0});
    }
    if (!player.ship.has_value()) {
        player.ship = spawnShip(registry, context, slot);
    }
}

void resetGame(engine::ecs::Registry &registry,
               engine::ecs::CommandBuffer &commands, GameContext &context)
{
    clearEntities(registry, commands, context);
    for (std::size_t slot = 0; slot < context.players.size(); ++slot) {
        if (context.players[slot].present) {
            resetPlayer(registry, context, static_cast<std::uint8_t>(slot));
        }
    }
    context.wave = WaveState{context.rules.enemySpawnInterval, 0};
    context.level = 1;
    context.hits.clear();
}

void updateGameOver(engine::ecs::Registry &registry,
                    engine::ecs::CommandBuffer &commands, GameContext &context,
                    bool anyone, float deltaTime)
{
    if (!anyone) {
        resetGame(registry, commands, context);
        setPhase(context, GamePhase::WAITING);
        return;
    }
    if (context.rules.gameOverDelay <= 0.0F) {
        return;
    }
    context.gameOverTimer -= deltaTime;
    if (context.gameOverTimer <= 0.0F) {
        resetGame(registry, commands, context);
        setPhase(context, GamePhase::PLAYING);
    }
}

void updatePhase(engine::ecs::Registry &registry,
                 engine::ecs::CommandBuffer &commands, GameContext &context,
                 float deltaTime)
{
    const bool anyone = anyPlayerPresent(context);

    if (context.phase == GamePhase::WAITING && anyone) {
        resetGame(registry, commands, context);
        setPhase(context, GamePhase::PLAYING);
    } else if (context.phase == GamePhase::PLAYING && !anyone) {
        resetGame(registry, commands, context);
        setPhase(context, GamePhase::WAITING);
    } else if (context.phase == GamePhase::GAME_OVER) {
        updateGameOver(registry, commands, context, anyone, deltaTime);
    }
}

} // namespace

void addGameStateSystem(engine::ecs::Scheduler &scheduler, GameContext &context)
{
    scheduler.addSystem(
        "game-state", engine::ecs::Phase::PRE_UPDATE, engine::ecs::Access{},
        [&context](engine::ecs::Registry &registry,
                   engine::ecs::CommandBuffer &commands, double deltaTime) {
            updatePhase(registry, commands, context,
                        static_cast<float>(deltaTime));
        });
}

} /* namespace rtype::game */
