/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameEvents.hpp
*/

#ifndef GAMEEVENTS_HPP_
#define GAMEEVENTS_HPP_

#include <cstdint>
#include <variant>

#include "Engine/Core/Vec2.hpp"

namespace rtype::game {

/**
 * @brief Owner or killer value meaning "no player" (enemies, the world).
 */
inline constexpr std::uint8_t NO_PLAYER = 0xFF;

/**
 * @brief What an entity is. Same values as the protocol spawn.kind.
 */
enum class EntityKind : std::uint8_t {
    PLAYER_SHIP = 1,
    BYDOS = 2,
    PLAYER_MISSILE = 3,
    BYDOS_MISSILE = 4
};

/**
 * @brief Why an entity was destroyed. Same values as destroy.reason.
 */
enum class DestroyReason : std::uint8_t {
    LEFT_SCREEN = 0,
    KILLED = 1,
    PICKED_UP = 2
};

/**
 * @brief Phase of the game. Same values as game_state.phase.
 */
enum class GamePhase : std::uint8_t {
    WAITING = 0,
    PLAYING = 1,
    GAME_OVER = 2
};

/**
 * @brief An entity appeared.
 */
struct SpawnEvent {
    std::uint16_t netId = 0;
    EntityKind kind = EntityKind::BYDOS;

    /** @brief Sub-type of the kind (monster type, weapon type). */
    std::uint8_t variant = 0;

    /** @brief Player slot owning the entity, or NO_PLAYER. */
    std::uint8_t owner = NO_PLAYER;

    /** @brief Position in pixels. */
    engine::core::Vec2 position;

    /** @brief Velocity in pixels per second. */
    engine::core::Vec2 velocity;
};

/**
 * @brief An entity disappeared.
 */
struct DestroyEvent {
    std::uint16_t netId = 0;
    DestroyReason reason = DestroyReason::LEFT_SCREEN;

    /** @brief Player slot credited with the kill, or NO_PLAYER. */
    std::uint8_t killer = NO_PLAYER;
};

/**
 * @brief The score of a player changed. It carries the new total.
 */
struct ScoreEvent {
    std::uint8_t player = 0;
    std::uint32_t score = 0;
};

/**
 * @brief The phase or the level of the game changed.
 */
struct GameStateEvent {
    GamePhase phase = GamePhase::WAITING;
    std::uint8_t level = 1;
};

/**
 * @brief Something the clients must be told reliably.
 */
using GameEvent =
    std::variant<SpawnEvent, DestroyEvent, ScoreEvent, GameStateEvent>;

/**
 * @brief Position of a living entity, for the snapshots.
 */
struct EntityState {
    std::uint16_t netId = 0;
    EntityKind kind = EntityKind::BYDOS;

    /** @brief Position in pixels. */
    engine::core::Vec2 position;
};

} /* namespace rtype::game */

#endif /* !GAMEEVENTS_HPP_ */
