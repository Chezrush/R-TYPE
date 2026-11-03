/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LobbyText.cpp
*/

#include "Client/Lobby/LobbyText.hpp"

#include <algorithm>

namespace client::lobby {

namespace {

const char *statusText(const events::RoomInfo &room) noexcept
{
    if (isRoomFull(room))
        return "Full";
    return room.status == events::RoomStatus::PLAYING ? "Playing" : "Waiting";
}

} // namespace

std::string roomRowText(const events::RoomInfo &room)
{
    const char *status = statusText(room);

    return room.name + "   " + std::to_string(room.players) + "/" +
           std::to_string(room.maxPlayers) + "   " + status;
}

bool isRoomFull(const events::RoomInfo &room) noexcept
{
    return room.players >= room.maxPlayers;
}

bool isValidRoomName(const std::string &name)
{
    const auto printable = [](char character) {
        return character >= 0x20 && character <= 0x7E;
    };

    return !name.empty() && name.size() <= MAX_ROOM_NAME &&
           std::all_of(name.begin(), name.end(), printable) &&
           name.find_first_not_of(' ') != std::string::npos;
}

std::string joinFailureMessage(events::JoinFailure reason)
{
    switch (reason) {
        case events::JoinFailure::ROOM_FULL:
            return "This room is full. Choose another one.";
        case events::JoinFailure::ROOM_NOT_FOUND:
            return "This room does not exist anymore. Refresh the list.";
        case events::JoinFailure::ALREADY_STARTED:
            return "The game has already started in this room.";
        case events::JoinFailure::INVALID_NAME:
            return "The server refused this room name.";
        case events::JoinFailure::TIMEOUT:
            return "The server did not answer. Try again.";
    }
    return "The server refused to join this room.";
}

std::string slotText(const events::LobbyUpdated &lobby, std::size_t slot)
{
    const events::LobbySlot &player = lobby.slots.at(slot);

    if (!player.occupied)
        return "Free";
    std::string text =
        player.nickname + (player.ready ? "   Ready" : "   Not ready");
    if (slot == lobby.hostPlayer)
        text += "   (host)";
    if (slot == lobby.localPlayer)
        text += "   (you)";
    return text;
}

bool canStart(const events::LobbyUpdated &lobby) noexcept
{
    const auto readyOrFree = [](const events::LobbySlot &slot) {
        return !slot.occupied || slot.ready;
    };

    return lobby.localPlayer == lobby.hostPlayer &&
           std::all_of(lobby.slots.begin(), lobby.slots.end(), readyOrFree);
}

} // namespace client::lobby
