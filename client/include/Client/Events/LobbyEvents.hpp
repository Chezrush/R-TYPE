/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LobbyEvents.hpp
*/

#ifndef LOBBYEVENTS_HPP_
#define LOBBYEVENTS_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace client::events {

/** @brief Players in a room. */
inline constexpr std::size_t LOBBY_SLOTS = 4;

/** @brief State of a room in the browser. */
enum class RoomStatus : std::uint8_t {
    WAITING,
    PLAYING
};

/** @brief One room of the server. */
struct RoomInfo {
    std::uint32_t id = 0;
    std::string name;
    std::uint8_t players = 0;
    std::uint8_t maxPlayers = 4;
    RoomStatus status = RoomStatus::WAITING;
};

/** @brief The browser asks for the rooms. */
struct RoomListRequested {};

/** @brief The rooms of the server. */
struct RoomListReceived {
    std::vector<RoomInfo> rooms;
};

/** @brief The player creates a room, and joins it. */
struct CreateRoomRequested {
    std::string name;
    std::uint8_t maxPlayers = 4;
};

/** @brief The player joins a room. */
struct JoinRoomRequested {
    std::uint32_t roomId = 0;
};

/** @brief The player leaves the room for the browser. */
struct LeaveRoomRequested {};

/**
 * @brief Why joining or creating a room failed.
 *
 * Open enumeration: an unknown value is kept and shown as a generic message.
 */
enum class JoinFailure : std::uint8_t {
    ROOM_FULL = 1,
    ROOM_NOT_FOUND = 2,
    ALREADY_STARTED = 3,
    INVALID_NAME = 4,
    TIMEOUT = 0xF0
};

/** @brief The server refused the room. */
struct JoinFailed {
    JoinFailure reason = JoinFailure::TIMEOUT;
};

/** @brief One player slot of the lobby. */
struct LobbySlot {
    bool occupied = false;
    std::string nickname;
    bool ready = false;
};

/** @brief The whole lobby, sent on every join, leave or ready change. */
struct LobbyUpdated {
    std::string roomName;
    std::array<LobbySlot, LOBBY_SLOTS> slots{};
    std::uint8_t localPlayer = 0;
    std::uint8_t hostPlayer = 0;
};

/** @brief The local player is ready, or not anymore. */
struct ReadyChanged {
    bool ready = false;
};

/** @brief The host starts the game. */
struct StartGameRequested {};

/** @brief A chat message from the local player. */
struct ChatMessageSent {
    std::string text;
};

/**
 * @brief A chat message from the server.
 *
 * player is NO_PLAYER (GameEvents.hpp) for a message from the server itself.
 */
struct ChatMessageReceived {
    std::uint8_t player = 0;
    std::string nickname;
    std::string text;
};

} // namespace client::events

#endif /* !LOBBYEVENTS_HPP_ */
