/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Messages.hpp
*/

#ifndef MESSAGES_HPP_
#define MESSAGES_HPP_

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "Protocol.hpp"

namespace rtype::protocol {

enum class RejectReason : std::uint8_t {
    SERVER_FULL = 1,
    INVALID_NAME = 2,
    VERSION = 3,
    RATE_LIMITED = 4,
    SHUTTING_DOWN = 5
};

enum class EntityKind : std::uint8_t {
    PLAYER_SHIP = 1,
    BYDOS = 2,
    PLAYER_MISSILE = 3,
    BYDOS_MISSILE = 4
};

enum class DestroyReason : std::uint8_t {
    EXPIRED = 0,
    KILLED = 1,
    PICKED_UP = 2
};

enum class GamePhase : std::uint8_t {
    WAITING = 0,
    PLAYING = 1,
    GAME_OVER = 2
};

/**
 * @brief Bit of the keys byte of an input sample.
 */
enum KeyBit : std::uint8_t {
    KEY_UP = 0x01,
    KEY_DOWN = 0x02,
    KEY_LEFT = 0x04,
    KEY_RIGHT = 0x08,
    KEY_SHOOT = 0x10
};

struct Hello {
    static constexpr MessageType TYPE = MessageType::HELLO;
    std::uint64_t cookie = 0;
    std::string name;

    friend bool operator==(const Hello &, const Hello &) = default;
};

struct Challenge {
    static constexpr MessageType TYPE = MessageType::CHALLENGE;
    std::uint64_t cookie = 0;

    friend bool operator==(const Challenge &, const Challenge &) = default;
};

struct Welcome {
    static constexpr MessageType TYPE = MessageType::WELCOME;
    std::uint8_t playerIndex = 0;
    std::uint8_t maxPlayers = 4;
    std::uint8_t tickRate = 60;
    std::uint8_t snapshotRate = 20;
    std::uint16_t worldWidth = 1280;
    std::uint16_t worldHeight = 720;
    std::uint16_t shipSpeed = 0;
    std::uint32_t serverTick = 0;

    friend bool operator==(const Welcome &, const Welcome &) = default;
};

struct Reject {
    static constexpr MessageType TYPE = MessageType::REJECT;
    std::uint8_t reason = 0;

    friend bool operator==(const Reject &, const Reject &) = default;
};

struct Goodbye {
    static constexpr MessageType TYPE = MessageType::GOODBYE;
    std::uint8_t reason = 0;

    friend bool operator==(const Goodbye &, const Goodbye &) = default;
};

struct Input {
    static constexpr MessageType TYPE = MessageType::INPUT;
    std::uint16_t firstId = 0;
    std::vector<std::uint8_t> keys;

    friend bool operator==(const Input &, const Input &) = default;
};

struct EntityState {
    std::uint16_t netId = 0;
    std::int16_t x = 0;
    std::int16_t y = 0;

    friend bool operator==(const EntityState &, const EntityState &) = default;
};

struct Snapshot {
    static constexpr MessageType TYPE = MessageType::SNAPSHOT;
    std::uint32_t tick = 0;
    std::uint16_t inputAck = 0;
    std::vector<EntityState> entities;

    friend bool operator==(const Snapshot &, const Snapshot &) = default;
};

struct Spawn {
    static constexpr MessageType TYPE = MessageType::SPAWN;
    std::uint16_t netId = 0;
    std::uint8_t kind = 0;
    std::uint8_t variant = 0;
    std::uint8_t owner = NO_PLAYER;
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t vx = 0;
    std::int16_t vy = 0;

    friend bool operator==(const Spawn &, const Spawn &) = default;
};

struct Destroy {
    static constexpr MessageType TYPE = MessageType::DESTROY;
    std::uint16_t netId = 0;
    std::uint8_t reason = 0;
    std::uint8_t killer = NO_PLAYER;

    friend bool operator==(const Destroy &, const Destroy &) = default;
};

struct PlayerJoined {
    static constexpr MessageType TYPE = MessageType::PLAYERJOINED;
    std::uint8_t playerIndex = 0;
    std::string name;

    friend bool operator==(const PlayerJoined &,
                           const PlayerJoined &) = default;
};

struct PlayerLeft {
    static constexpr MessageType TYPE = MessageType::PLAYERLEFT;
    std::uint8_t playerIndex = 0;
    std::uint8_t reason = 0;

    friend bool operator==(const PlayerLeft &, const PlayerLeft &) = default;
};

struct Score {
    static constexpr MessageType TYPE = MessageType::SCORE;
    std::uint8_t playerIndex = 0;
    std::uint32_t score = 0;

    friend bool operator==(const Score &, const Score &) = default;
};

struct GameState {
    static constexpr MessageType TYPE = MessageType::GAMESTATE;
    std::uint8_t phase = 0;
    std::uint8_t level = 0;

    friend bool operator==(const GameState &, const GameState &) = default;
};

using Message =
    std::variant<Hello, Challenge, Welcome, Reject, Goodbye, Input, Snapshot,
                 Spawn, Destroy, PlayerJoined, PlayerLeft, Score, GameState>;

/**
 * @brief Returns true if the type is carried by the reliable channel.
 */
bool isReliable(MessageType type) noexcept;
bool isReliable(std::uint8_t type) noexcept;

/**
 * @brief Returns the wire type of a message.
 */
MessageType typeOf(const Message &message);

} /* namespace rtype::protocol */

#endif /* !MESSAGES_HPP_ */
