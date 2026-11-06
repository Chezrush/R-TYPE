/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Players.hpp
*/

#ifndef PLAYERS_HPP_
#define PLAYERS_HPP_

#include <cstdint>

#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Whether a player is still in the game.
 *
 * A player is in the game while the ship is alive or a life is left.
 */
[[nodiscard]] bool hasActivePlayers(const GameContext &context);

/**
 * @brief Ends the game if every player present is out of lives.
 *
 * Sets the phase to GAME_OVER and emits a GameStateEvent, once. Does nothing
 * when nobody is present.
 */
void checkGameOver(GameContext &context);

/**
 * @brief Gives points to a player and emits the new total as a ScoreEvent.
 *
 * A slot without a player is ignored.
 */
void addScore(GameContext &context, std::uint8_t slot, std::uint32_t points);

/**
 * @brief Records that the ship of a slot was destroyed.
 *
 * The player loses a life. If one is left the respawn timer starts,
 * otherwise the game may be over.
 */
void onShipDestroyed(GameContext &context, std::uint8_t slot);

} /* namespace rtype::game */

#endif /* !PLAYERS_HPP_ */
