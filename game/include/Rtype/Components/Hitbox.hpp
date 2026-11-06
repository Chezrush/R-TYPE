/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Hitbox.hpp
*/

#ifndef HITBOX_HPP_
#define HITBOX_HPP_

namespace rtype::game {

/**
 * @brief Axis-aligned rectangle used for collisions, in pixels.
 *
 * The rectangle is centred on the Position of the entity.
 */
struct Hitbox {
    float width = 0.0F;
    float height = 0.0F;

    friend bool operator==(const Hitbox &, const Hitbox &) = default;
};

} /* namespace rtype::game */

#endif /* !HITBOX_HPP_ */
