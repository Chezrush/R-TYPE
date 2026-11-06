/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Position.hpp
*/

#ifndef POSITION_HPP_
#define POSITION_HPP_

#include "Engine/Core/Vec2.hpp"

namespace rtype::game {

/**
 * @brief Centre of an entity in the world, in pixels.
 *
 * The origin is the top-left corner of the world, x grows to the right and y
 * grows downwards.
 */
struct Position {
    engine::core::Vec2 value;

    friend bool operator==(const Position &, const Position &) = default;
};

} /* namespace rtype::game */

#endif /* !POSITION_HPP_ */
