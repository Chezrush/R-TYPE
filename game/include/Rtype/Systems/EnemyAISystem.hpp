/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EnemyAISystem.hpp
*/

#ifndef ENEMYAISYSTEM_HPP_
#define ENEMYAISYSTEM_HPP_

#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Adds the system that drives the enemies, before the movement.
 *
 * An enemy with the STRAIGHT pattern flies left at constant speed. One with
 * the SINE_WAVE pattern also swings up and down. An enemy that is on the
 * screen shoots a missile every rules.enemyFireInterval seconds. The registry
 * and the context must outlive the scheduler.
 */
void addEnemyAISystem(engine::ecs::Scheduler &scheduler,
                      engine::ecs::Registry &registry, GameContext &context);

} /* namespace rtype::game */

#endif /* !ENEMYAISYSTEM_HPP_ */
