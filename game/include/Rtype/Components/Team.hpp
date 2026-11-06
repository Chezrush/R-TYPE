/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Team.hpp
*/

#ifndef TEAM_HPP_
#define TEAM_HPP_

#include <cstdint>

namespace rtype::game {

/**
 * @brief The two sides of the game. Entities of a side never hit each other.
 */
enum class TeamSide : std::uint8_t {
    PLAYERS = 0,
    ENEMIES = 1
};

/**
 * @brief Side of an entity, used by the collisions.
 */
struct Team {
    TeamSide side = TeamSide::ENEMIES;

    friend bool operator==(const Team &, const Team &) = default;
};

} /* namespace rtype::game */

#endif /* !TEAM_HPP_ */
