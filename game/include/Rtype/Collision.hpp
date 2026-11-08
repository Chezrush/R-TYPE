/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Collision.hpp
*/

#ifndef COLLISION_HPP_
#define COLLISION_HPP_

#include "Engine/Core/Vec2.hpp"
#include "Rtype/Components/Hitbox.hpp"

namespace rtype::game {

/**
 * @brief Whether two hitboxes, each centred on its position, overlap.
 *
 * Rectangles that only touch by an edge do not overlap.
 */
[[nodiscard]] bool boxesOverlap(const engine::core::Vec2 &firstPosition,
                                const Hitbox &first,
                                const engine::core::Vec2 &secondPosition,
                                const Hitbox &second) noexcept;

} /* namespace rtype::game */

#endif /* !COLLISION_HPP_ */
