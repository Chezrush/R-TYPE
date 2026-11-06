/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** PlayerState.hpp
*/

#ifndef PLAYERSTATE_HPP_
#define PLAYERSTATE_HPP_

#include <cstdint>
#include <optional>

#include "Engine/Ecs/Entity.hpp"

namespace rtype::game {

/**
 * @brief Everything the game keeps about one player slot.
 *
 * The ship is an entity and dies, the player stays: so the lives, the score,
 * the respawn timer and the keys held live here and not on the ship.
 */
struct PlayerState {
    /** @brief Whether a player was added in this slot and did not leave. */
    bool present = false;

    /** @brief The ship, or nothing while the player waits to respawn. */
    std::optional<engine::ecs::Entity> ship{};

    /** @brief Lives left, counting the one being played. */
    std::uint8_t lives = 0;

    std::uint32_t score = 0;

    /** @brief Seconds left before the ship comes back. */
    float respawnTimer = 0.0F;

    /** @brief Keys held, given to the next ship at once. */
    std::uint8_t keys = 0;
};

} /* namespace rtype::game */

#endif /* !PLAYERSTATE_HPP_ */
