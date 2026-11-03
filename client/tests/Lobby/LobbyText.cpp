/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LobbyText.cpp
*/

#include "Client/Lobby/LobbyText.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

using client::events::JoinFailure;
using client::events::LobbyUpdated;
using client::events::RoomInfo;
using client::events::RoomStatus;
using namespace client::lobby;

namespace {

LobbyUpdated twoPlayers()
{
    LobbyUpdated lobby;

    lobby.roomName = "Room";
    lobby.slots[0] = {true, "Ann", true};
    lobby.slots[1] = {true, "Bob", false};
    lobby.localPlayer = 1;
    lobby.hostPlayer = 0;
    return lobby;
}

} // namespace

TEST_CASE("a room row shows name, players and status", "[client][lobby]")
{
    REQUIRE(roomRowText({1, "Bob's room", 2, 4, RoomStatus::WAITING}) ==
            "Bob's room   2/4   Waiting");
    REQUIRE(roomRowText({2, "Fight", 3, 4, RoomStatus::PLAYING}) ==
            "Fight   3/4   Playing");
    REQUIRE(roomRowText({3, "Full", 2, 2, RoomStatus::WAITING}) ==
            "Full   2/2   Full");
}

TEST_CASE("room names are 1 to 24 plain characters", "[client][lobby]")
{
    REQUIRE(isValidRoomName("Bob's room"));
    REQUIRE(isValidRoomName(std::string(24, 'a')));
    REQUIRE_FALSE(isValidRoomName(""));
    REQUIRE_FALSE(isValidRoomName("   "));
    REQUIRE_FALSE(isValidRoomName(std::string(25, 'a')));
    REQUIRE_FALSE(isValidRoomName("Caf\xC3\xA9"));
}

TEST_CASE("every join failure has its own message", "[client][lobby]")
{
    const std::string full = joinFailureMessage(JoinFailure::ROOM_FULL);

    REQUIRE(full.find("full") != std::string::npos);
    for (const JoinFailure other :
         {JoinFailure::ROOM_NOT_FOUND, JoinFailure::ALREADY_STARTED,
          JoinFailure::INVALID_NAME, JoinFailure::TIMEOUT,
          static_cast<JoinFailure>(77)}) {
        REQUIRE_FALSE(joinFailureMessage(other).empty());
        REQUIRE(joinFailureMessage(other) != full);
    }
}

TEST_CASE("a slot shows nickname, ready state, host and you", "[client][lobby]")
{
    const LobbyUpdated lobby = twoPlayers();

    REQUIRE(slotText(lobby, 0) == "Ann   Ready   (host)");
    REQUIRE(slotText(lobby, 1) == "Bob   Not ready   (you)");
    REQUIRE(slotText(lobby, 2) == "Free");
}

TEST_CASE("only the host starts, once everyone is ready", "[client][lobby]")
{
    LobbyUpdated lobby = twoPlayers();
    REQUIRE_FALSE(canStart(lobby));

    lobby.localPlayer = 0;
    REQUIRE_FALSE(canStart(lobby));
    lobby.slots[1].ready = true;
    REQUIRE(canStart(lobby));
}
