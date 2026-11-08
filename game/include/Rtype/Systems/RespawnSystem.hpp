/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RespawnSystem.hpp
*/

#ifndef RESPAWNSYSTEM_HPP_
#define RESPAWNSYSTEM_HPP_

#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Adds the system that brings the dead players back, before the others.
 *
 * A player without a ship and with a life left waits rules.respawnDelay
 * seconds, then gets a new ship (and its SpawnEvent) holding the keys that
 * were last given to setInput(). The context must outlive the scheduler.
 */
void addRespawnSystem(engine::ecs::Scheduler &scheduler, GameContext &context);

} /* namespace rtype::game */

#endif /* !RESPAWNSYSTEM_HPP_ */
