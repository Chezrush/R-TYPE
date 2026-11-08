/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Bounds.cpp
*/

#include "Rtype/Bounds.hpp"

namespace rtype::game {

bool isOutsideWorld(const engine::core::Vec2 &position, const Hitbox &hitbox,
                    const GameRules &rules) noexcept
{
    const float halfWidth = hitbox.width / 2.0F;
    const float halfHeight = hitbox.height / 2.0F;

    return position.x + halfWidth < 0.0F ||
           position.x - halfWidth > rules.worldWidth ||
           position.y + halfHeight < 0.0F ||
           position.y - halfHeight > rules.worldHeight;
}

} /* namespace rtype::game */
