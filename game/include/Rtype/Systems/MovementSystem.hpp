/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MovementSystem.hpp
*/

#ifndef MOVEMENTSYSTEM_HPP_
#define MOVEMENTSYSTEM_HPP_

#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameRules.hpp"

namespace rtype::game {

/**
 * @brief Adds the systems that move the entities.
 *
 * The first one adds velocity * deltaTime to every position. The second one
 * keeps the player ships inside the world, so together they give the same
 * result as moveShip(). The rules must outlive the scheduler.
 */
void addMovementSystems(engine::ecs::Scheduler &scheduler,
                        const GameRules &rules);

} /* namespace rtype::game */

#endif /* !MOVEMENTSYSTEM_HPP_ */
