/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** DamageSystem.hpp
*/

#ifndef DAMAGESYSTEM_HPP_
#define DAMAGESYSTEM_HPP_

#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Adds the system that turns the hits into damage and deaths.
 *
 * It runs right after the collision system and reads context.hits. Both
 * entities of a hit lose one health point. An entity that reaches 0 is
 * destroyed with the reason KILLED, credited to the owner of the other
 * entity. An entity is destroyed at most once per tick, and the first hit in
 * the list wins.
 *
 * - A destroyed enemy gives rules.enemyScore points to its killer.
 * - A destroyed player ship costs its player a life, see onShipDestroyed().
 *
 * The context must outlive the scheduler.
 */
void addDamageSystem(engine::ecs::Scheduler &scheduler, GameContext &context);

} /* namespace rtype::game */

#endif /* !DAMAGESYSTEM_HPP_ */
