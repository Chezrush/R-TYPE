/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EnemyAI.hpp
*/

#ifndef ENEMYAI_HPP_
#define ENEMYAI_HPP_

#include <cstdint>

namespace rtype::game {

/**
 * @brief How an enemy moves. More patterns come with the enemies task.
 */
enum class EnemyPattern : std::uint8_t {
    STRAIGHT = 0,
    SINE_WAVE = 1
};

/**
 * @brief State of the behavior of an enemy.
 */
struct EnemyAI {
    EnemyPattern pattern = EnemyPattern::STRAIGHT;

    /** @brief Seconds since the enemy appeared, drives the pattern. */
    float age = 0.0F;

    /** @brief Seconds left before the enemy can shoot again. */
    float fireCooldown = 0.0F;

    friend bool operator==(const EnemyAI &, const EnemyAI &) = default;
};

} /* namespace rtype::game */

#endif /* !ENEMYAI_HPP_ */
