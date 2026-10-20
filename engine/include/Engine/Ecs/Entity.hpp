/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Entity.hpp
*/

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include <compare>
#include <cstddef>
#include <cstdint>

namespace engine::ecs {

class EntityAllocator;

/**
 * @brief Handle to an entity: an index and a generation.
 *
 * The index addresses the component arrays. The generation changes each time
 * an index is reused, so a handle kept after its entity died can be detected.
 * Only EntityAllocator can create a valid handle.
 */
class Entity {
public:
    /** @brief Index of the entity, usable to address component arrays. */
    [[nodiscard]] std::uint32_t index() const noexcept;

    /** @brief Number of times the index was reused before this entity. */
    [[nodiscard]] std::uint32_t generation() const noexcept;

    /** @brief Index as std::size_t; ignores the generation. */
    [[nodiscard]] operator std::size_t() const noexcept;

    [[nodiscard]] bool operator==(const Entity &) const noexcept = default;
    [[nodiscard]] auto operator<=>(const Entity &) const noexcept = default;

private:
    friend class EntityAllocator;

    explicit Entity(std::uint32_t index, std::uint32_t generation) noexcept;

    std::uint32_t _index;
    std::uint32_t _generation;
};

static_assert(sizeof(Entity) == 8);

} // namespace engine::ecs

#endif /* !ENTITY_HPP_ */
