/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Lives.hpp
*/

#ifndef LIVES_HPP_
#define LIVES_HPP_

#include <cstdint>

namespace rtype::game {

/**
 * @brief Lives a player has left. The player is out at 0.
 */
struct Lives {
    std::uint8_t remaining = 3;

    friend bool operator==(const Lives &, const Lives &) = default;
};

} /* namespace rtype::game */

#endif /* !LIVES_HPP_ */
