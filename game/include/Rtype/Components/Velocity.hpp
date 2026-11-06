/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Velocity.hpp
*/

#ifndef VELOCITY_HPP_
#define VELOCITY_HPP_

#include "Engine/Core/Vec2.hpp"

namespace rtype::game {

/**
 * @brief Speed of an entity, in pixels per second.
 */
struct Velocity {
    engine::core::Vec2 value;

    friend bool operator==(const Velocity &, const Velocity &) = default;
};

} /* namespace rtype::game */

#endif /* !VELOCITY_HPP_ */
