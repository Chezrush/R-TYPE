/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientSession.cpp
*/

#include "Client/Net/ClientSession.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <variant>

#include "Client/World/WorldState.hpp"
#include "Server.hpp"

using client::net::ClientSession;
using namespace std::chrono_literals;
namespace proto = rtype::protocol;

namespace {

rtype::server::ServerConfig config()
{
    rtype::server::ServerConfig value;

    value.port = 0;
    return value;
}

bool waitFor(const std::function<bool()> &condition)
{
    const auto deadline = std::chrono::steady_clock::now() + 4s;

    while (!condition()) {
        if (std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::sleep_for(5ms);
    }
    return true;
}

/**
 * @brief A session with the world it feeds, driven like the game loop does.
 */
struct Player {
    ClientSession session;
    client::world::WorldState world;
    std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();

    Player(std::uint16_t port, const std::string &name)
        : session("127.0.0.1", port, name)
    {}

    double now() const
    {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
            .count();
    }

    void pump()
    {
        session.update(std::chrono::steady_clock::now());
        if (!world.configured() && session.welcome()) {
            world.configure(*session.welcome());
        }
        for (const auto &message : session.takeMessages()) {
            world.apply(message, now());
        }
    }
};

std::size_t shipCount(const Player &player)
{
    const auto items = player.world.visible(player.now());

    return static_cast<std::size_t>(
        std::count_if(items.begin(), items.end(),
                      [](const auto &item) { return item.kind == 1; }));
}

bool hasPlayer(const Player &player, const std::string &name)
{
    const auto &players = player.world.players();

    return std::any_of(players.begin(), players.end(),
                       [&name](const auto &info) {
                           return info.present && info.name == name;
                       });
}

std::size_t presentCount(const Player &player)
{
    const auto &players = player.world.players();

    return static_cast<std::size_t>(
        std::count_if(players.begin(), players.end(),
                      [](const auto &info) { return info.present; }));
}

} // namespace

TEST_CASE("a session connects to the server and receives a welcome")
{
    rtype::server::Server server(config());
    REQUIRE(server.start());
    Player player(server.port(), "Bob");

    REQUIRE(waitFor([&] {
        player.pump();
        return player.session.state() == ClientSession::State::CONNECTED &&
               player.world.configured();
    }));
    CHECK(player.world.localSlot() == 0);
    CHECK(waitFor([&] { return server.playerCount() == 1; }));
    server.shutdown();
}

TEST_CASE("the own ship appears and moves with the keys sent")
{
    rtype::server::Server server(config());
    REQUIRE(server.start());
    Player player(server.port(), "Bob");
    float firstX = -1.0F;

    REQUIRE(waitFor([&] {
        player.pump();
        const auto ships = player.world.visible(player.now());

        if (!ships.empty() && firstX < 0.0F) {
            firstX = ships[0].position.x;
        }
        return !ships.empty();
    }));
    REQUIRE(waitFor([&] {
        const std::uint8_t keys = proto::KEY_RIGHT;

        player.world.predict(
            player.session.sendInput(keys, std::chrono::steady_clock::now()),
            keys);
        std::this_thread::sleep_for(16ms);
        player.pump();
        return player.world.visible(player.now())[0].position.x >
               firstX + 30.0F;
    }));
    server.shutdown();
}

TEST_CASE("two players see each other")
{
    rtype::server::Server server(config());
    REQUIRE(server.start());
    Player first(server.port(), "Bob");
    Player second(server.port(), "Eve");

    REQUIRE(waitFor([&] {
        first.pump();
        second.pump();
        return shipCount(first) == 2 && shipCount(second) == 2 &&
               hasPlayer(first, "Eve") && hasPlayer(second, "Bob");
    }));
    server.shutdown();
}

TEST_CASE("a leaving player disappears for the others")
{
    rtype::server::Server server(config());
    REQUIRE(server.start());
    auto first = std::make_unique<Player>(server.port(), "Bob");
    Player second(server.port(), "Eve");

    REQUIRE(waitFor([&] {
        first->pump();
        second.pump();
        return shipCount(second) == 2;
    }));
    first->session.disconnect(std::chrono::steady_clock::now());
    REQUIRE(waitFor([&] {
        second.pump();
        return shipCount(second) == 1 && presentCount(second) == 0;
    }));
    server.shutdown();
}

TEST_CASE("a full server refuses the next player")
{
    rtype::server::Server server(config());
    REQUIRE(server.start());
    std::vector<std::unique_ptr<Player>> players;

    for (int i = 0; i < 4; ++i) {
        players.push_back(
            std::make_unique<Player>(server.port(), "P" + std::to_string(i)));
    }
    REQUIRE(waitFor([&] {
        bool all = true;

        for (auto &player : players) {
            player->pump();
            all = all &&
                  player->session.state() == ClientSession::State::CONNECTED;
        }
        return all;
    }));
    Player extra(server.port(), "Late");

    REQUIRE(waitFor([&] {
        extra.pump();
        return extra.session.state() == ClientSession::State::REJECTED;
    }));
    CHECK(extra.session.rejectReason() ==
          static_cast<std::uint8_t>(proto::RejectReason::SERVER_FULL));
    server.shutdown();
}

TEST_CASE("an unresolvable host fails at once")
{
    ClientSession session("host.invalid", 4242, "Bob");

    CHECK(session.state() == ClientSession::State::FAILED);
    CHECK(session.sendInput(1, std::chrono::steady_clock::now()) == 0);
}

TEST_CASE("the session fails when the server never answers")
{
    ClientSession session("127.0.0.1", 9, "Bob");
    const auto start = std::chrono::steady_clock::now();

    session.update(start);
    session.update(start + 11s);
    CHECK(session.state() == ClientSession::State::FAILED);
}
