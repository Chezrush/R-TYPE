/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameRules.hpp
*/

#ifndef GAMERULES_HPP_
#define GAMERULES_HPP_

#include <cstdint>

namespace rtype::game {

/**
 * @brief Bits of the keys byte given to Simulation::setInput().
 *
 * Same layout as the protocol keys bitmask. Bits 5 to 7 are always 0.
 */
namespace keys {

inline constexpr std::uint8_t UP = 1U << 0U;
inline constexpr std::uint8_t DOWN = 1U << 1U;
inline constexpr std::uint8_t LEFT = 1U << 2U;
inline constexpr std::uint8_t RIGHT = 1U << 3U;
inline constexpr std::uint8_t SHOOT = 1U << 4U;

} /* namespace keys */

/**
 * @brief Rules of one game. The server sends some of them in welcome.
 *
 * Distances are in pixels, speeds in pixels per second, durations in
 * seconds.
 */
struct GameRules {
    /** @brief Simulation ticks per second. step() advances 1 / tickRate. */
    std::uint32_t tickRate = 60;

    /** @brief Width of the world, in pixels. */
    float worldWidth = 1280.0F;

    /** @brief Height of the world, in pixels. */
    float worldHeight = 720.0F;

    /** @brief Speed of a player ship, in pixels per second. */
    float shipSpeed = 300.0F;

    /** @brief Number of player slots, from 0 to maxPlayers - 1. */
    std::uint8_t maxPlayers = 4;

    /** @brief Lives of a player at the start of a game. */
    std::uint8_t lives = 3;

    /** @brief Time before a dead player respawns, in seconds. */
    double respawnDelay = 2.0;
};

} /* namespace rtype::game */

#endif /* !GAMERULES_HPP_ */
