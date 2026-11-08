/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Waves.cpp
*/

#include "Rtype/Waves.hpp"

#include <algorithm>

namespace rtype::game {

namespace {

constexpr std::uint64_t MAX_LEVEL = 255;
constexpr float SPEED_PER_LEVEL = 0.1F;

} // namespace

std::uint8_t levelAfter(std::uint32_t enemiesSpawned,
                        const GameRules &rules) noexcept
{
    if (rules.enemiesPerLevel == 0) {
        return 1;
    }
    const std::uint64_t level = 1ULL + enemiesSpawned / rules.enemiesPerLevel;

    return static_cast<std::uint8_t>(std::min(level, MAX_LEVEL));
}

float enemySpeed(const GameRules &rules, std::uint8_t level) noexcept
{
    const float extra = static_cast<float>(level - 1) * SPEED_PER_LEVEL;

    return rules.enemySpeed * (1.0F + extra);
}

} /* namespace rtype::game */
