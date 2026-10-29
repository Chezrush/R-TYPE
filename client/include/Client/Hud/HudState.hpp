/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudState.hpp
*/

#ifndef HUDSTATE_HPP_
#define HUDSTATE_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "Client/Events/GameEvents.hpp"

namespace client::hud {

/** @brief Players per game (protocol player_index is 0..3). */
inline constexpr std::size_t MAX_PLAYERS = 4;

/** @brief Lives shown before the first death. */
inline constexpr int STARTING_LIVES = 3;

/**
 * @brief Everything the HUD displays, as plain data.
 */
struct HudState {
    std::optional<std::uint8_t> localPlayer;
    std::optional<std::uint16_t> localShip;
    bool localAlive = false;
    int lives = STARTING_LIVES;
    std::uint32_t score = 0;
    std::uint32_t level = 1;
    events::GamePhase phase = events::GamePhase::WAITING;
    float beamCharge = 0.0f;
    std::array<bool, MAX_PLAYERS> connected{};

    friend bool operator==(const HudState &, const HudState &) = default;
};

} // namespace client::hud

#endif /* !HUDSTATE_HPP_ */
