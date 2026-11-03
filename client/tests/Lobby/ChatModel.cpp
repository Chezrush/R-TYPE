/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ChatModel.cpp
*/

#include "Client/Lobby/ChatModel.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "Client/Events/GameEvents.hpp"

using client::events::ChatMessageReceived;
using client::lobby::ChatCommand;
using client::lobby::ChatModel;

TEST_CASE("chat lines have a timestamp and an author", "[client][chat]")
{
    ChatModel model;

    model.add({1, "Bob", "hi"}, 205.9);
    model.add({client::events::NO_PLAYER, "", "Ann joined"}, 3600.0);

    REQUIRE(model.lines() == std::vector<std::string>{"[03:25] Bob: hi",
                                                      "[60:00] * Ann joined"});
}

TEST_CASE("typed lines are sent, trimmed and cut", "[client][chat]")
{
    REQUIRE(ChatModel::parse("   ").kind == ChatCommand::Kind::NOTHING);
    const ChatCommand hello = ChatModel::parse("  hello there");
    REQUIRE(hello.kind == ChatCommand::Kind::SEND);
    REQUIRE(hello.text == "hello there");
    REQUIRE(ChatModel::parse(std::string(300, 'a')).text.size() ==
            client::lobby::MAX_CHAT_LENGTH);
}

TEST_CASE("mute commands use the player numbers", "[client][chat]")
{
    const ChatCommand mute = ChatModel::parse("/mute 2");
    REQUIRE(mute.kind == ChatCommand::Kind::MUTE);
    REQUIRE(mute.player == 1);
    REQUIRE(ChatModel::parse("/unmute 4").kind == ChatCommand::Kind::UNMUTE);
    REQUIRE(ChatModel::parse("/mute 5").kind == ChatCommand::Kind::INVALID);
    REQUIRE(ChatModel::parse("/mute x").kind == ChatCommand::Kind::INVALID);
    REQUIRE(ChatModel::parse("/muted").kind == ChatCommand::Kind::SEND);
}

TEST_CASE("a muted player is hidden until unmuted", "[client][chat]")
{
    ChatModel model;
    model.add({1, "Bob", "spam"}, 1.0);
    model.add({0, "Ann", "hello"}, 2.0);

    model.setMuted(1, true);
    REQUIRE(model.lines() == std::vector<std::string>{"[00:02] Ann: hello"});
    model.setMuted(1, false);
    REQUIRE(model.lines().size() == 2);
}

TEST_CASE("the chat keeps the latest lines", "[client][chat]")
{
    ChatModel model;

    for (std::size_t i = 0; i < ChatModel::MAX_LINES + 3; ++i)
        model.add({0, "Ann", std::to_string(i)}, 0.0);

    REQUIRE(model.lines().size() == ChatModel::MAX_LINES);
    REQUIRE(model.lines().front() == "[00:00] Ann: 3");
}
