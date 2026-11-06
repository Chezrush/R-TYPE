/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetIdAllocator.hpp
*/

#ifndef NETIDALLOCATOR_HPP_
#define NETIDALLOCATOR_HPP_

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

namespace rtype::game {

/**
 * @brief Hands out the 16-bit net_id of the networked entities.
 *
 * The protocol wants a net_id unique among the living entities and reused
 * only after its destroy. On top of that, the allocator delays the reuse as
 * long as it can, so a client that is late on a destroy cannot confuse an old
 * entity with a new one:
 * - ids never used yet are handed out first, in increasing order;
 * - only when all of them were used, released ids come back in the order they
 *   were released (oldest first).
 *
 * Every value from 0 to 65535 is a valid id. Not thread-safe: owned by the
 * simulation thread, like the rest of the game.
 */
class NetIdAllocator {
public:
    /** @brief Number of different ids. */
    static constexpr std::size_t CAPACITY = 65536;

    /**
     * @brief Takes an id that no living entity uses.
     *
     * @return The id, or std::nullopt when all CAPACITY ids are in use.
     */
    [[nodiscard]] std::optional<std::uint16_t> allocate();

    /**
     * @brief Gives an id back once its entity is destroyed.
     *
     * @return true if the id was in use, false if it was not (released twice
     * or never allocated). Nothing changes in that case.
     */
    bool release(std::uint16_t id);

    /** @brief Whether an entity currently uses the id. */
    [[nodiscard]] bool isAllocated(std::uint16_t id) const noexcept;

    /** @brief Number of ids in use. */
    [[nodiscard]] std::size_t aliveCount() const noexcept;

private:
    std::vector<bool> _inUse = std::vector<bool>(CAPACITY, false);
    std::deque<std::uint16_t> _released;
    std::size_t _nextFresh = 0;
    std::size_t _alive = 0;
};

} /* namespace rtype::game */

#endif /* !NETIDALLOCATOR_HPP_ */
