/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CollisionSystem.hpp
*/

#ifndef COLLISIONSYSTEM_HPP_
#define COLLISIONSYSTEM_HPP_

#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Adds the system that finds which entities touch each other.
 *
 * Every pair of entities from opposite teams whose hitboxes overlap becomes a
 * Hit in context.hits, which it empties first. Entities of the same team
 * never collide. The pairs are in the order of the entity indices, so the
 * result is the same for the same game. The context must outlive the
 * scheduler.
 */
void addCollisionSystem(engine::ecs::Scheduler &scheduler,
                        GameContext &context);

} /* namespace rtype::game */

#endif /* !COLLISIONSYSTEM_HPP_ */
