/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Collision.cpp
*/

#include "Rtype/Collision.hpp"

#include <cmath>

namespace rtype::game {

bool boxesOverlap(const engine::core::Vec2 &firstPosition, const Hitbox &first,
                  const engine::core::Vec2 &secondPosition,
                  const Hitbox &second) noexcept
{
    const float reachX = (first.width + second.width) / 2.0F;
    const float reachY = (first.height + second.height) / 2.0F;

    return std::fabs(firstPosition.x - secondPosition.x) < reachX &&
           std::fabs(firstPosition.y - secondPosition.y) < reachY;
}

} /* namespace rtype::game */
