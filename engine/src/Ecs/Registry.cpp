/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Registry.cpp
*/

#include "Engine/Ecs/Registry.hpp"

namespace engine::ecs {

Entity Registry::spawnEntity()
{
    return _entities.create();
}

bool Registry::killEntity(Entity entity) noexcept
{
    return _entities.destroy(entity);
}

bool Registry::isAlive(Entity entity) const noexcept
{
    return _entities.isAlive(entity);
}

std::size_t Registry::aliveCount() const noexcept
{
    return _entities.aliveCount();
}

} // namespace engine::ecs
