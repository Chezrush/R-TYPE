/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** InputSystem.cpp
*/

#include "Rtype/Systems/InputSystem.hpp"

#include <algorithm>

#include "Rtype/Components/Components.hpp"
#include "Rtype/Entities.hpp"
#include "Rtype/ShipMovement.hpp"

namespace rtype::game {

namespace {

bool wantsToShoot(const PlayerControl &control) noexcept
{
    return (control.keys & keys::SHOOT) != 0 && control.fireCooldown <= 0.0F;
}

} // namespace

void addInputSystem(engine::ecs::Scheduler &scheduler,
                    engine::ecs::Registry &registry, GameContext &context)
{
    scheduler.addSystem<PlayerControl, Velocity, const Position>(
        "input", engine::ecs::Phase::PRE_UPDATE,
        [&registry, &context](engine::ecs::Entity, PlayerControl &control,
                              Velocity &velocity, const Position &position,
                              engine::ecs::CommandBuffer &commands,
                              double deltaTime) {
            const float cooldown =
                control.fireCooldown - static_cast<float>(deltaTime);

            velocity.value =
                shipVelocity(control.keys, context.rules.shipSpeed);
            control.fireCooldown = std::max(0.0F, cooldown);
            if (wantsToShoot(control) &&
                spawnMissile(registry, commands, context, position,
                             control.slot)) {
                control.fireCooldown = context.rules.fireInterval;
            }
        });
}

} /* namespace rtype::game */
