/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RespawnSystem.cpp
*/

#include "Rtype/Systems/RespawnSystem.hpp"

#include "Rtype/Entities.hpp"

namespace rtype::game {

namespace {

bool isWaiting(const PlayerState &player) noexcept
{
    return player.present && !player.ship.has_value() && player.lives > 0;
}

void respawn(engine::ecs::Registry &registry, GameContext &context,
             float deltaTime)
{
    for (std::size_t slot = 0; slot < context.players.size(); ++slot) {
        PlayerState &player = context.players[slot];

        if (!isWaiting(player)) {
            continue;
        }
        player.respawnTimer -= deltaTime;
        if (player.respawnTimer <= 0.0F) {
            player.ship =
                spawnShip(registry, context, static_cast<std::uint8_t>(slot));
        }
    }
}

} // namespace

void addRespawnSystem(engine::ecs::Scheduler &scheduler, GameContext &context)
{
    scheduler.addSystem(
        "respawn", engine::ecs::Phase::PRE_UPDATE, engine::ecs::Access{},
        [&context](engine::ecs::Registry &registry,
                   engine::ecs::CommandBuffer &, double deltaTime) {
            respawn(registry, context, static_cast<float>(deltaTime));
        });
}

} /* namespace rtype::game */
