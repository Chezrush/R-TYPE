/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Waves.hpp
*/

#ifndef WAVES_HPP_
#define WAVES_HPP_

#include <cstdint>

#include "Rtype/GameRules.hpp"

namespace rtype::game {

/**
 * @brief Progress of the enemy waves of one game.
 */
struct WaveState {
    /** @brief Seconds left before the next enemy appears. */
    float spawnTimer = 0.0F;

    /** @brief Enemies created since the start of the game. */
    std::uint32_t enemiesSpawned = 0;
};

/**
 * @brief Level reached once some enemies have appeared.
 *
 * The game starts at level 1 and goes up one level every
 * rules.enemiesPerLevel enemies, up to 255. Zero means the level never
 * changes.
 */
[[nodiscard]] std::uint8_t levelAfter(std::uint32_t enemiesSpawned,
                                      const GameRules &rules) noexcept;

/**
 * @brief Speed of an enemy created at a level, in pixels per second.
 *
 * Each level adds 10 % of rules.enemySpeed.
 */
[[nodiscard]] float enemySpeed(const GameRules &rules,
                               std::uint8_t level) noexcept;

} /* namespace rtype::game */

#endif /* !WAVES_HPP_ */
