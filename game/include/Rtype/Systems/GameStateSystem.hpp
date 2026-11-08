/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameStateSystem.hpp
*/

#ifndef GAMESTATESYSTEM_HPP_
#define GAMESTATESYSTEM_HPP_

#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Adds the system that moves the game between its phases, first of a
 * tick.
 *
 * - WAITING to PLAYING: as soon as a player is in the game.
 * - PLAYING to WAITING: when nobody is left.
 * - GAME_OVER (set by checkGameOver()) to PLAYING: after
 *   rules.gameOverDelay seconds, if players are still there. If nobody is
 *   left it goes to WAITING at once. A delay of 0 never starts a new game.
 *
 * Starting or stopping a game resets the world: enemies and missiles are
 * destroyed, the level goes back to 1, the waves restart, and every player
 * present gets the lives of rules.lives, a score of 0 and a ship. The context
 * must outlive the scheduler.
 */
void addGameStateSystem(engine::ecs::Scheduler &scheduler,
                        GameContext &context);

} /* namespace rtype::game */

#endif /* !GAMESTATESYSTEM_HPP_ */
