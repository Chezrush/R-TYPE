/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ShipMovement.cpp
*/

#include "Rtype/ShipMovement.hpp"

#include <algorithm>

namespace rtype::game {

namespace {

float axis(std::uint8_t pressed, std::uint8_t positive,
           std::uint8_t negative) noexcept
{
    float value = 0.0F;

    if ((pressed & positive) != 0) {
        value += 1.0F;
    }
    if ((pressed & negative) != 0) {
        value -= 1.0F;
    }
    return value;
}

} // namespace

engine::core::Vec2 shipVelocity(std::uint8_t pressed, float speed) noexcept
{
    const engine::core::Vec2 direction{axis(pressed, keys::RIGHT, keys::LEFT),
                                       axis(pressed, keys::DOWN, keys::UP)};

    return engine::core::normalized(direction) * speed;
}

engine::core::Vec2 clampToWorld(const engine::core::Vec2 &position,
                                const GameRules &rules) noexcept
{
    return {std::clamp(position.x, 0.0F, rules.worldWidth),
            std::clamp(position.y, 0.0F, rules.worldHeight)};
}

engine::core::Vec2 moveShip(const engine::core::Vec2 &position,
                            std::uint8_t pressed, const GameRules &rules,
                            float deltaTime) noexcept
{
    const engine::core::Vec2 velocity = shipVelocity(pressed, rules.shipSpeed);

    return clampToWorld(position + velocity * deltaTime, rules);
}

} /* namespace rtype::game */
