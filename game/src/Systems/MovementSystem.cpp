/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MovementSystem.cpp
*/

#include "Rtype/Systems/MovementSystem.hpp"

#include "Rtype/Components/Components.hpp"
#include "Rtype/ShipMovement.hpp"

namespace rtype::game {

void addMovementSystems(engine::ecs::Scheduler &scheduler,
                        const GameRules &rules)
{
    scheduler.addSystem<Position, const Velocity>(
        "movement", engine::ecs::Phase::UPDATE,
        [](engine::ecs::Entity, Position &position, const Velocity &velocity,
           engine::ecs::CommandBuffer &, double deltaTime) {
            position.value =
                position.value + velocity.value * static_cast<float>(deltaTime);
        });
    scheduler.addSystem<Position, const PlayerControl>(
        "ship-bounds", engine::ecs::Phase::UPDATE,
        [&rules](engine::ecs::Entity, Position &position, const PlayerControl &,
                 engine::ecs::CommandBuffer &, double) {
            position.value = clampToWorld(position.value, rules);
        });
}

} /* namespace rtype::game */
