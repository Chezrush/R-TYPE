/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Phases.hpp
*/

#ifndef PHASES_HPP_
#define PHASES_HPP_

#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Changes the phase of the game and reports it.
 *
 * A GameStateEvent with the new phase and the current level is emitted. Does
 * nothing if the phase is already the one asked. Entering GAME_OVER starts
 * the timer of rules.gameOverDelay.
 */
void setPhase(GameContext &context, GamePhase phase);

} /* namespace rtype::game */

#endif /* !PHASES_HPP_ */
