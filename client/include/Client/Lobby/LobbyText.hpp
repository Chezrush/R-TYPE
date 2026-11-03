/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LobbyText.hpp
*/

#ifndef LOBBYTEXT_HPP_
#define LOBBYTEXT_HPP_

#include <cstddef>
#include <string>

#include "Client/Events/LobbyEvents.hpp"

namespace client::lobby {

/** @brief Longest room name. */
inline constexpr std::size_t MAX_ROOM_NAME = 24;

/** @brief "Bob's room   2/4   Waiting" */
[[nodiscard]] std::string roomRowText(const events::RoomInfo &room);

[[nodiscard]] bool isRoomFull(const events::RoomInfo &room) noexcept;

/** @brief 1 to 24 printable ASCII characters, not only spaces. */
[[nodiscard]] bool isValidRoomName(const std::string &name);

/** @brief Readable message for a failure, unknown reasons included. */
[[nodiscard]] std::string joinFailureMessage(events::JoinFailure reason);

/** @brief "Bob   Ready   (host)", or "Free" for an empty slot. */
[[nodiscard]] std::string slotText(const events::LobbyUpdated &lobby,
                                   std::size_t slot);

/** @brief The local player is the host and every player present is ready. */
[[nodiscard]] bool canStart(const events::LobbyUpdated &lobby) noexcept;

} // namespace client::lobby

#endif /* !LOBBYTEXT_HPP_ */
