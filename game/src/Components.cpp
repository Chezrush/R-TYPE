/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Components.cpp
*/

#include "Rtype/Components/Components.hpp"

namespace rtype::game {

void registerComponents(engine::ecs::Registry &registry)
{
    registry.registerComponent<Position>();
    registry.registerComponent<Velocity>();
    registry.registerComponent<Hitbox>();
    registry.registerComponent<NetId>();
    registry.registerComponent<Kind>();
    registry.registerComponent<PlayerControl>();
    registry.registerComponent<Health>();
    registry.registerComponent<Lives>();
    registry.registerComponent<Team>();
    registry.registerComponent<EnemyAI>();
}

} /* namespace rtype::game */
