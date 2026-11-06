/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** PlayerControl.hpp
*/

#ifndef PLAYERCONTROL_HPP_
#define PLAYERCONTROL_HPP_

#include <cstdint>

namespace rtype::game {

/**
 * @brief Marks a ship driven by a player, and holds what the player asks.
 */
struct PlayerControl {
    /** @brief Player slot, from 0 to maxPlayers - 1. */
    std::uint8_t slot = 0;

    /** @brief Keys held, see rtype::game::keys. Kept until changed. */
    std::uint8_t keys = 0;

    /** @brief Seconds left before the ship can shoot again. */
    float fireCooldown = 0.0F;

    friend bool operator==(const PlayerControl &,
                           const PlayerControl &) = default;
};

} /* namespace rtype::game */

#endif /* !PLAYERCONTROL_HPP_ */
