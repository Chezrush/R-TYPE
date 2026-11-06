/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Bounds.hpp
*/

#ifndef BOUNDS_HPP_
#define BOUNDS_HPP_

#include "Engine/Core/Vec2.hpp"
#include "Rtype/Components/Hitbox.hpp"
#include "Rtype/GameRules.hpp"

namespace rtype::game {

/**
 * @brief Whether a rectangle is entirely outside the world.
 *
 * The rectangle is the hitbox centred on the position. One that is partly
 * visible, or only touches the edge of the world, is not outside.
 */
[[nodiscard]] bool isOutsideWorld(const engine::core::Vec2 &position,
                                  const Hitbox &hitbox,
                                  const GameRules &rules) noexcept;

} /* namespace rtype::game */

#endif /* !BOUNDS_HPP_ */
