/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EntityAllocator.hpp
*/

#ifndef ENTITYALLOCATOR_HPP_
#define ENTITYALLOCATOR_HPP_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Engine/Ecs/Entity.hpp"

namespace engine::ecs {

/**
 * @brief Creates and destroys entities and tells whether a handle is alive.
 *
 * Each index has a generation that grows when its entity is destroyed, so a
 * handle kept after that moment is never alive again, even if the index is
 * reused. Freed indices are recycled in LIFO order: the most recently freed
 * index is the first one reused. A generation wraps around after 2^32 reuses
 * of the same index.
 */
class EntityAllocator {
public:
    /**
     * @brief Creates a live entity, reusing a freed index when there is one.
     * @throws std::length_error when every index is in use.
     */
    [[nodiscard]] Entity create();

    /**
     * @brief Destroys an entity.
     * @return true if the entity was alive, false for a stale or unknown
     * handle (nothing changes in that case).
     */
    bool destroy(Entity entity) noexcept;

    /** @brief Whether the handle designates an entity that was not destroyed.
     */
    [[nodiscard]] bool isAlive(Entity entity) const noexcept;

    /** @brief Number of live entities. */
    [[nodiscard]] std::size_t aliveCount() const noexcept;

private:
    std::vector<std::uint32_t> _generations;
    std::vector<std::uint32_t> _freeIndices;
};

} // namespace engine::ecs

#endif /* !ENTITYALLOCATOR_HPP_ */
