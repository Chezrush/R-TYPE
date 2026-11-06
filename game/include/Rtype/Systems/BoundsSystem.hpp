/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** BoundsSystem.hpp
*/

#ifndef BOUNDSSYSTEM_HPP_
#define BOUNDSSYSTEM_HPP_

#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Adds the system that destroys what left the screen.
 *
 * Runs after the movement. Every entity that is entirely outside the world is
 * destroyed with the reason LEFT_SCREEN and no killer. Player ships are never
 * destroyed by it: the movement keeps them inside the world. The context must
 * outlive the scheduler.
 */
void addBoundsSystem(engine::ecs::Scheduler &scheduler, GameContext &context);

} /* namespace rtype::game */

#endif /* !BOUNDSSYSTEM_HPP_ */
