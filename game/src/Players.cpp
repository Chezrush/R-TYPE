/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Players.cpp
*/

#include "Rtype/Players.hpp"

namespace rtype::game {

namespace {

bool isActive(const PlayerState &player) noexcept
{
    return player.present && (player.ship.has_value() || player.lives > 0);
}

} // namespace

bool hasActivePlayers(const GameContext &context)
{
    for (const PlayerState &player : context.players) {
        if (isActive(player)) {
            return true;
        }
    }
    return false;
}

void checkGameOver(GameContext &context)
{
    bool anyone = false;

    if (context.phase == GamePhase::GAME_OVER) {
        return;
    }
    for (const PlayerState &player : context.players) {
        anyone = anyone || player.present;
    }
    if (anyone && !hasActivePlayers(context)) {
        context.phase = GamePhase::GAME_OVER;
        context.events.emplace_back(
            GameStateEvent{GamePhase::GAME_OVER, context.level});
    }
}

void addScore(GameContext &context, std::uint8_t slot, std::uint32_t points)
{
    if (slot >= context.players.size() || !context.players[slot].present) {
        return;
    }
    PlayerState &player = context.players[slot];

    player.score += points;
    context.events.emplace_back(ScoreEvent{slot, player.score});
}

void onShipDestroyed(GameContext &context, std::uint8_t slot)
{
    if (slot >= context.players.size()) {
        return;
    }
    PlayerState &player = context.players[slot];

    player.ship.reset();
    if (player.lives > 0) {
        --player.lives;
    }
    if (player.lives > 0) {
        player.respawnTimer = static_cast<float>(context.rules.respawnDelay);
    }
    checkGameOver(context);
}

} /* namespace rtype::game */
