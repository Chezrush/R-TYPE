/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Health.hpp
*/

#ifndef HEALTH_HPP_
#define HEALTH_HPP_

#include <cstdint>

namespace rtype::game {

/**
 * @brief Hit points of an entity. It dies when current reaches 0.
 */
struct Health {
    std::int32_t current = 1;
    std::int32_t max = 1;

    friend bool operator==(const Health &, const Health &) = default;
};

} /* namespace rtype::game */

#endif /* !HEALTH_HPP_ */
