/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameEvents.hpp
*/

#ifndef GAMEEVENTS_HPP_
#define GAMEEVENTS_HPP_

#include <cstdint>
#include <string>

namespace client::events {

/** @brief `owner` / `killer` value meaning "no player". */
inline constexpr std::uint8_t NO_PLAYER = 0xFF;

/**
 * @brief Entity kinds of the protocol (`spawn.kind`).
 *
 * The protocol enumerations are open: an unknown value is kept as is.
 */
enum class EntityKind : std::uint8_t {
    PLAYER_SHIP = 1,
    BYDOS = 2,
    PLAYER_MISSILE = 3,
    BYDOS_MISSILE = 4
};

/** @brief Why an entity disappeared (`destroy.reason`). */
enum class DestroyReason : std::uint8_t {
    GONE = 0,
    KILLED = 1,
    PICKED_UP = 2
};

/** @brief Phase of the game (`game_state.phase`). */
enum class GamePhase : std::uint8_t {
    WAITING = 0,
    PLAYING = 1,
    GAME_OVER = 2
};

/**
 * @brief The server accepted us (`welcome`), sent once per connection.
 *
 * Our ship is not in it: it is the `spawn` whose kind is PLAYER_SHIP and
 * whose owner is playerIndex.
 */
struct WelcomeReceived {
    std::uint8_t playerIndex = 0;
    std::uint8_t maxPlayers = 4;
};

/** @brief An entity appeared (`spawn`), our ship included on respawn. */
struct EntitySpawned {
    std::uint16_t netId = 0;
    EntityKind kind = EntityKind::BYDOS;
    std::uint8_t owner = NO_PLAYER;
};

/** @brief An entity was removed (`destroy`). Its net_id may be reused. */
struct EntityDestroyed {
    std::uint16_t netId = 0;
    DestroyReason reason = DestroyReason::GONE;
    std::uint8_t killer = NO_PLAYER;
};

/** @brief Someone plays in a slot (`player_joined`). */
struct PlayerJoined {
    std::uint8_t playerIndex = 0;
    std::string name;
};

/** @brief A player quit or crashed (`player_left`, goodbye reasons). */
struct PlayerLeft {
    std::uint8_t playerIndex = 0;
    std::uint8_t reason = 0;
};

/** @brief New total score of a player (`score`). */
struct ScoreChanged {
    std::uint8_t playerIndex = 0;
    std::uint32_t score = 0;
};

/** @brief Phase or level of the game changed (`game_state`). */
struct GameStateChanged {
    GamePhase phase = GamePhase::WAITING;
    std::uint8_t level = 1;
};

} // namespace client::events

#endif /* !GAMEEVENTS_HPP_ */
