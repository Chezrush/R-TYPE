/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Phases.cpp
*/

#include "Rtype/Phases.hpp"

namespace rtype::game {

void setPhase(GameContext &context, GamePhase phase)
{
    if (context.phase == phase) {
        return;
    }
    context.phase = phase;
    if (phase == GamePhase::GAME_OVER) {
        context.gameOverTimer = context.rules.gameOverDelay;
    }
    context.events.emplace_back(GameStateEvent{phase, context.level});
}

} /* namespace rtype::game */
