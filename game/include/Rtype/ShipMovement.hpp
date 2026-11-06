/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ShipMovement.hpp
*/

#ifndef SHIPMOVEMENT_HPP_
#define SHIPMOVEMENT_HPP_

#include <cstdint>

#include "Engine/Core/Vec2.hpp"
#include "Rtype/GameRules.hpp"

namespace rtype::game {

/**
 * @brief Velocity of a ship holding some keys, in pixels per second.
 *
 * Opposite keys cancel each other. A diagonal has the same speed as a
 * straight line. Pure: it depends on nothing else, so the client can use it
 * to predict its own ship.
 */
[[nodiscard]] engine::core::Vec2 shipVelocity(std::uint8_t pressed,
                                              float speed) noexcept;

/**
 * @brief Moves a position back inside the world if it is outside.
 */
[[nodiscard]] engine::core::Vec2
clampToWorld(const engine::core::Vec2 &position,
             const GameRules &rules) noexcept;

/**
 * @brief Position of a ship after holding some keys during one step.
 *
 * It is exactly what the server does to a ship each tick, so a client that
 * calls it with its own keys stays in step with the server.
 *
 * @param deltaTime Duration of the step, in seconds.
 */
[[nodiscard]] engine::core::Vec2 moveShip(const engine::core::Vec2 &position,
                                          std::uint8_t pressed,
                                          const GameRules &rules,
                                          float deltaTime) noexcept;

} /* namespace rtype::game */

#endif /* !SHIPMOVEMENT_HPP_ */
