/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** BoundsSystem.cpp
*/

#include "Rtype/Systems/BoundsSystem.hpp"

#include "Rtype/Bounds.hpp"
#include "Rtype/Components/Components.hpp"
#include "Rtype/Entities.hpp"

namespace rtype::game {

void addBoundsSystem(engine::ecs::Scheduler &scheduler, GameContext &context)
{
    scheduler.addSystem<const Position, const Hitbox, const Kind, const NetId>(
        "bounds", engine::ecs::Phase::POST_UPDATE,
        [&context](engine::ecs::Entity entity, const Position &position,
                   const Hitbox &hitbox, const Kind &kind, const NetId &netId,
                   engine::ecs::CommandBuffer &commands, double) {
            if (kind.kind != EntityKind::PLAYER_SHIP &&
                isOutsideWorld(position.value, hitbox, context.rules)) {
                announceDestroy(context, netId.value,
                                DestroyReason::LEFT_SCREEN, NO_PLAYER);
                commands.kill(entity);
            }
        });
}

} /* namespace rtype::game */
