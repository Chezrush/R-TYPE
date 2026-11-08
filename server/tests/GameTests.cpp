/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** GameTests
*/

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"
#include "Rtype/GameRules.hpp"
#include "Server.hpp"
#include "ServerTestSupport.hpp"
#include "TestSupport.hpp"

using namespace rtype::server;
using namespace rtype::protocol;
using namespace std::chrono_literals;

namespace game = rtype::game;

namespace {

constexpr std::uint64_t TICKS_TO_MOVE = 20;

std::string packetOf(const Message &message)
{
    std::string packet;

    PacketBuilder::writeHeader(packet, MessageHeader{});
    MessageCodec::appendMessage(packet, message);
    return packet;
}

std::optional<game::SpawnEvent> shipOf(const Server &server, std::uint8_t slot)
{
    const auto world = server.simulation().currentWorld();
    const auto ship =
        std::ranges::find_if(world, [slot](const game::SpawnEvent &entity) {
            return entity.kind == game::EntityKind::PLAYER_SHIP &&
                   entity.owner == slot;
        });

    if (ship == world.end()) {
        return std::nullopt;
    }
    return *ship;
}

/**
 * @brief Two clients join the started server, as the Gateway delivers them.
 */
void joinTwoClients(Server &server)
{
    CHECK(server.start());
    CHECK(server.receivePacket({1, packetOf(Hello{0, "Alice"}), {}, 0}));
    CHECK(server.receivePacket({2, packetOf(Hello{0, "Bob"}), {}, 1}));
    CHECK(waitFor([&] { return server.playerCount() == 2; }));
}

/**
 * @brief Restarts the server, the first client goes right and the second
 * goes down for a few ticks.
 */
void moveShips(Server &server)
{
    const std::vector<std::uint8_t> right(MAX_INPUTS, game::keys::RIGHT);
    const Input down{0, {game::keys::DOWN}};

    CHECK(server.start());
    const auto start = server.tickCount();
    CHECK(server.receivePacket({1, packetOf(Input{0, right}), {}, {}}));
    CHECK(server.receivePacket({2, packetOf(down), {}, {}}));
    CHECK(waitFor([&] { return server.tickCount() >= start + TICKS_TO_MOVE; }));
    server.shutdown();
}

void testInputsMoveTheShips()
{
    Server server(testConfig(5000ms));

    joinTwoClients(server);
    server.shutdown();
    const auto first = shipOf(server, 0);
    const auto second = shipOf(server, 1);
    moveShips(server);
    const auto movedFirst = shipOf(server, 0);
    const auto movedSecond = shipOf(server, 1);

    CHECK(first && second && movedFirst && movedSecond);
    if (!first || !second || !movedFirst || !movedSecond) {
        return;
    }
    CHECK(movedFirst->position.x > first->position.x);
    CHECK(movedFirst->position.y == first->position.y);
    CHECK(movedSecond->position.x == second->position.x);
    CHECK(movedSecond->position.y > second->position.y);
}

void testDisconnectionRemovesTheShip()
{
    Server server(testConfig(5000ms));
    std::string goodbye;

    PacketBuilder::writeHeader(goodbye, MessageHeader{});
    PacketBuilder::appendGoodbye(goodbye, DisconnectReason::QUIT);
    joinTwoClients(server);
    CHECK(server.receivePacket({1, goodbye, {}, {}}));
    CHECK(waitFor([&] { return server.playerCount() == 1; }));
    server.shutdown();

    CHECK(!shipOf(server, 0));
    CHECK(shipOf(server, 1));
}

} // namespace

int main()
{
    testInputsMoveTheShips();
    testDisconnectionRemovesTheShip();

    return TEST_RESULT();
}
