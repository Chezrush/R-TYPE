/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Registry.hpp
*/

#ifndef REGISTRY_HPP_
#define REGISTRY_HPP_

#include <cstddef>

#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/EntityAllocator.hpp"

namespace engine::ecs {

/**
 * @brief Single entry point of the ECS: owns the entities.
 *
 * The registry is the only owner of its data, so it cannot be copied.
 */
class Registry {
public:
    Registry() = default;
    Registry(const Registry &) = delete;
    Registry &operator=(const Registry &) = delete;
    Registry(Registry &&) noexcept = default;
    Registry &operator=(Registry &&) noexcept = default;
    ~Registry() = default;

    /**
     * @brief Creates a live entity, reusing a freed index when possible.
     * @throws std::length_error when every index is in use.
     */
    [[nodiscard]] Entity spawnEntity();

    /**
     * @brief Destroys an entity.
     * @return true if the entity was alive, false for a stale or unknown
     * handle (nothing changes in that case).
     */
    bool killEntity(Entity entity) noexcept;

    /** @brief Whether the handle designates an entity that was not killed.
     */
    [[nodiscard]] bool isAlive(Entity entity) const noexcept;

    /** @brief Number of live entities. */
    [[nodiscard]] std::size_t aliveCount() const noexcept;

private:
    EntityAllocator _entities;
};

} // namespace engine::ecs

#endif /* !REGISTRY_HPP_ */
