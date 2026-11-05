/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** DisconnectionTests.cpp
*/

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "ClientActivity.hpp"
#include "PlayerSessions.hpp"
#include "Protocol/PacketReader.hpp"
#include "Server.hpp"
#include "TestSupport.hpp"

using namespace rtype::server;
using namespace rtype::protocol;
using namespace std::chrono_literals;

namespace {

using Bytes = std::vector<std::uint8_t>;

const Bytes HEADER = {0x52, 0x54, 0x01, 0x00, 0, 0, 0, 0,
                      0,    0,    0,    0,    0, 0, 0, 0};

std::string packet(const Bytes &records)
{
    Bytes bytes = HEADER;

    bytes.insert(bytes.end(), records.begin(), records.end());
    return {bytes.begin(), bytes.end()};
}

const std::string HELLO =
    packet({0x01, 0x00, 0x0C, 0, 0, 0, 0, 0, 0, 0, 0, 0x03, 0x42, 0x6F, 0x62});
const std::string GOODBYE = packet({0x05, 0x00, 0x01, 0x00});

ServerConfig testConfig(std::chrono::milliseconds timeout)
{
    ServerConfig config;

    config.port = 0;
    config.inactivityTimeout = timeout;
    return config;
}

/**
 * @brief Polls the condition until it holds or one second elapsed.
 */
bool waitFor(const std::function<bool()> &condition)
{
    const auto deadline = std::chrono::steady_clock::now() + 1s;

    while (!condition()) {
        if (std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::sleep_for(5ms);
    }
    return true;
}

void testActivityExpiresAfterTimeout()
{
    ClientActivity activity;
    const auto start = ClientActivity::Clock::now();

    activity.touch(1, start);
    activity.touch(2, start + 400ms);

    CHECK(activity.takeExpired(start + 500ms, 500ms).empty());
    CHECK(activity.takeExpired(start + 600ms, 500ms) ==
          std::vector<std::uint32_t>{1});
    CHECK(activity.takeExpired(start + 600ms, 500ms).empty());
    CHECK(activity.forget(2));
    CHECK(!activity.forget(2));
}

void testLeaveNotifiesOtherPlayers()
{
    PlayerSessions players;

    CHECK(players.join(1));
    CHECK(players.join(2));
    CHECK(!players.join(2));

    const auto notices = players.leave(1, DisconnectReason::TIMEOUT);

    CHECK(notices.size() == 1);
    if (notices.empty()) {
        return;
    }
    const std::string &payload = notices.front().payload;

    CHECK(notices.front().clientId == 2);
    CHECK(PacketReader::isValid(payload));
    CHECK(PacketReader::hasRecord(payload, MessageType::PLAYERLEFT));
    CHECK(payload[21] == 0);
    CHECK(payload[22] == static_cast<char>(DisconnectReason::TIMEOUT));
    CHECK(players.size() == 1);
}

void testRepeatedLeaveIsIgnored()
{
    PlayerSessions players;

    CHECK(players.join(1));
    CHECK(players.join(2));
    CHECK(players.leave(1, DisconnectReason::QUIT).size() == 1);
    CHECK(players.leave(1, DisconnectReason::QUIT).empty());
    CHECK(players.leave(1, DisconnectReason::TIMEOUT).empty());
    CHECK(players.size() == 1);
    CHECK(players.join(1));
}

void testExplicitDisconnection()
{
    Server server(testConfig(5000ms));

    CHECK(server.start());
    CHECK(server.receivePacket({1, HELLO, {}}));
    CHECK(server.receivePacket({2, HELLO, {}}));
    CHECK(waitFor([&] { return server.playerCount() == 2; }));

    CHECK(server.receivePacket({1, GOODBYE, {}}));
    CHECK(server.receivePacket({1, GOODBYE, {}}));
    CHECK(waitFor([&] { return server.playerCount() == 1; }));
    server.shutdown();
}

void testTimeoutDisconnection()
{
    Server server(testConfig(100ms));

    CHECK(server.start());
    CHECK(server.receivePacket({1, HELLO, {}}));
    CHECK(waitFor([&] { return server.playerCount() == 1; }));

    const auto ticksBefore = server.tickCount();

    CHECK(waitFor([&] { return server.playerCount() == 0; }));
    CHECK(server.tickCount() > ticksBefore);
    CHECK(server.isRunning());
    server.shutdown();
}

} /* namespace */

int main()
{
    testActivityExpiresAfterTimeout();
    testLeaveNotifiesOtherPlayers();
    testRepeatedLeaveIsIgnored();
    testExplicitDisconnection();
    testTimeoutDisconnection();

    return TEST_RESULT();
}
