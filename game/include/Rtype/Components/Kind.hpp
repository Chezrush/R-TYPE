/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Kind.hpp
*/

#ifndef KIND_HPP_
#define KIND_HPP_

#include <cstdint>

#include "Rtype/GameEvents.hpp"

namespace rtype::game {

/**
 * @brief What an entity is, and who owns it.
 */
struct Kind {
    EntityKind kind = EntityKind::BYDOS;

    /** @brief Sub-type of the kind (monster type, weapon type). */
    std::uint8_t variant = 0;

    /** @brief Player slot owning the entity, or NO_PLAYER. */
    std::uint8_t owner = NO_PLAYER;

    friend bool operator==(const Kind &, const Kind &) = default;
};

} /* namespace rtype::game */

#endif /* !KIND_HPP_ */
