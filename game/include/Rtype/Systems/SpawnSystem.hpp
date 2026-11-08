/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SpawnSystem.hpp
*/

#ifndef SPAWNSYSTEM_HPP_
#define SPAWNSYSTEM_HPP_

#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Adds the system that creates the enemies, before the other systems.
 *
 * While the game is in the PLAYING phase, an enemy appears every
 * rules.enemySpawnInterval seconds on the right of the screen (never if it is
 * 0). Each new level
 * is announced by a GameStateEvent. The height and the pattern of the enemies
 * come from the seed of the game. The context must outlive the scheduler.
 */
void addSpawnSystem(engine::ecs::Scheduler &scheduler, GameContext &context);

} /* namespace rtype::game */

#endif /* !SPAWNSYSTEM_HPP_ */
