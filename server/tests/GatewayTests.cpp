/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GatewayTests.cpp
*/

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ClientHandshake.hpp"
#include "Gateway.hpp"
#include "PacketBuilder.hpp"
#include "PacketReader.hpp"
#include "TestSupport.hpp"

using namespace rtype::server;
using namespace rtype::protocol;
using namespace std::chrono_literals;

namespace {

struct Harness {
    std::vector<std::pair<std::string, std::string>> sent;
    std::vector<IncomingMessage> delivered;
    std::vector<std::pair<std::uint32_t, DisconnectReason>> dropped;
    Gateway gateway;

    Harness()
        : gateway(
              GatewayConfig{}, SipKey{},
              [this](const std::string &to, const std::string &data) {
                  sent.emplace_back(to, data);
              },
              [this](IncomingMessage message) {
                  delivered.push_back(std::move(message));
                  return true;
              },
              [this](std::uint32_t id, DisconnectReason reason) {
                  dropped.emplace_back(id, reason);
              })
    {}

    std::vector<std::string> takeSentTo(const std::string &address)
    {
        std::vector<std::string> mine;
        std::vector<std::pair<std::string, std::string>> others;

        for (auto &item : sent) {
            if (item.first == address) {
                mine.push_back(item.second);
            } else {
                others.push_back(item);
            }
        }
        sent.clear();
        sent = std::move(others);
        return mine;
    }
};

struct TestClient {
    std::string address;
    ClientHandshake handshake;
    std::unique_ptr<Connection> connection;
    std::vector<Message> reliable;

    TestClient(std::string where, const std::string &name, TimePoint now)
        : address(std::move(where)),
          handshake(name, now)
    {}

    void listen(Harness &harness, TimePoint now)
    {
        for (const std::string &datagram : harness.takeSentTo(address)) {
            if (connection) {
                const auto result = connection->receive(datagram, now);

                reliable.insert(reliable.end(), result.reliable.begin(),
                                result.reliable.end());
            } else {
                handshake.onDatagram(datagram);
                adopt(datagram);
            }
        }
    }

    void adopt(const std::string &datagram)
    {
        if (handshake.state() == ClientHandshake::State::CONNECTED) {
            connection = std::make_unique<Connection>(handshake.session());
            const auto result = connection->receive(datagram, Clock::now());

            reliable.insert(reliable.end(), result.reliable.begin(),
                            result.reliable.end());
        }
    }

    void send(Harness &harness, const std::string &datagram, TimePoint now)
    {
        harness.gateway.onDatagram(address, datagram, now, 1000);
    }
};

bool connect(Harness &harness, TestClient &client, TimePoint &now)
{
    for (int step = 0; step < 20; ++step, now += 600ms) {
        if (const auto hello = client.handshake.poll(now)) {
            client.send(harness, *hello, now);
        }
        client.listen(harness, now);
        if (client.connection) {
            return true;
        }
    }
    return false;
}

void testClientConnectsAndGetsAWelcome()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient client("1.1.1.1|1000", "Bob", now);

    CHECK(connect(harness, client, now));
    CHECK(harness.gateway.clientCount() == 1);
    CHECK(client.handshake.welcome() &&
          client.handshake.welcome()->playerIndex == 0);
    CHECK(harness.delivered.size() == 1);
    CHECK(harness.delivered[0].slot == 0);
    CHECK(PacketReader::hasRecord(harness.delivered[0].payload,
                                  MessageType::HELLO));
}

void testPlayersAreIntroducedToEachOther()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient first("1.1.1.1|1000", "Bob", now);
    TestClient second("2.2.2.2|2000", "Eve", now);

    CHECK(connect(harness, first, now));
    CHECK(connect(harness, second, now));
    harness.gateway.flush(now + 300ms);
    first.listen(harness, now + 300ms);
    second.listen(harness, now + 300ms);
    CHECK(second.handshake.welcome()->playerIndex == 1);
    CHECK(first.reliable.size() == 1);
    CHECK(first.reliable[0] == Message(PlayerJoined{1, "Eve"}));
    CHECK(second.reliable.size() == 1);
    CHECK(second.reliable[0] == Message(PlayerJoined{0, "Bob"}));
}

void testSlotsRunOut()
{
    Harness harness;
    TimePoint now = Clock::now();
    std::vector<std::unique_ptr<TestClient>> clients;

    for (int i = 0; i < 5; ++i) {
        clients.push_back(std::make_unique<TestClient>(
            "9.9.9.9|" + std::to_string(i), "P" + std::to_string(i), now));
    }
    for (int i = 0; i < 4; ++i) {
        CHECK(connect(harness, *clients[i], now));
    }
    CHECK(!connect(harness, *clients[4], now));
    CHECK(clients[4]->handshake.state() == ClientHandshake::State::REJECTED);
    CHECK(clients[4]->handshake.rejectReason() ==
          static_cast<std::uint8_t>(RejectReason::SERVER_FULL));
}

bool handshakeAgain(Harness &harness, TestClient &client, TimePoint now,
                    std::uint64_t nowSeconds, ClientHandshake &again)
{
    harness.gateway.onDatagram(client.address, *again.poll(now), now,
                               nowSeconds);
    for (const std::string &reply : harness.takeSentTo(client.address)) {
        again.onDatagram(reply);
    }
    const auto second = again.poll(now + 1ms);

    if (!second) {
        return false;
    }
    harness.gateway.onDatagram(client.address, *second, now + 1ms, nowSeconds);
    for (const std::string &reply : harness.takeSentTo(client.address)) {
        again.onDatagram(reply);
    }
    return true;
}

void testRepeatedHelloGetsTheSameWelcomeAgain()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient client("1.1.1.1|1000", "Bob", now);
    ClientHandshake again("Bob", now);

    CHECK(connect(harness, client, now));
    harness.sent.clear();
    CHECK(handshakeAgain(harness, client, now, 1000, again));
    CHECK(again.state() == ClientHandshake::State::CONNECTED);
    CHECK(again.session() == client.handshake.session());
    CHECK(harness.gateway.clientCount() == 1);
    CHECK(harness.dropped.empty());
}

void testNewCookieReplacesTheConnection()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient client("1.1.1.1|1000", "Bob", now);
    ClientHandshake restarted("Bob", now);

    CHECK(connect(harness, client, now));
    CHECK(handshakeAgain(harness, client, now, 1100, restarted));
    CHECK(restarted.state() == ClientHandshake::State::CONNECTED);
    CHECK(restarted.session() != client.handshake.session());
    CHECK(harness.gateway.clientCount() == 1);
    CHECK(harness.dropped.size() == 1);
    CHECK(harness.dropped[0].second == DisconnectReason::QUIT);
}

void testGoodbyeRemovesTheClient()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient client("1.1.1.1|1000", "Bob", now);

    CHECK(connect(harness, client, now));
    const std::vector<Message> goodbye = {Goodbye{0}};

    client.send(harness, client.connection->buildDatagram(now, goodbye), now);
    CHECK(harness.gateway.clientCount() == 0);
    CHECK(PacketReader::hasRecord(harness.delivered.back().payload,
                                  MessageType::GOODBYE));
}

void testStrangerDatagramsAreIgnored()
{
    Harness harness;
    const TimePoint now = Clock::now();
    Connection stranger(0x1234);

    harness.gateway.onDatagram("6.6.6.6|1", stranger.buildDatagram(now), now,
                               1000);
    harness.gateway.onDatagram("6.6.6.6|1", "junk", now, 1000);
    CHECK(harness.sent.empty());
    CHECK(harness.gateway.clientCount() == 0);
    CHECK(harness.delivered.empty());
}

void testInvalidFloodDropsTheClient()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient client("1.1.1.1|1000", "Bob", now);

    CHECK(connect(harness, client, now));
    for (int i = 0; i < 60; ++i) {
        harness.gateway.onDatagram(client.address, "garbage!garbage!garbage!",
                                   now, 1000);
    }
    CHECK(harness.gateway.clientCount() == 0);
    CHECK(harness.dropped.size() == 1);
    CHECK(harness.dropped[0].second == DisconnectReason::PROTOCOL);
}

void testSimulationMessagesReachTheClient()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient client("1.1.1.1|1000", "Bob", now);
    std::string payload;

    CHECK(connect(harness, client, now));
    PacketBuilder::writeHeader(payload, MessageHeader{});
    PacketBuilder::appendPlayerLeft(payload, 0, 3, DisconnectReason::TIMEOUT);
    harness.gateway.queueFromSimulation(
        OutgoingMessage{harness.delivered[0].clientId, payload});
    harness.gateway.flush(now);
    client.listen(harness, now);
    CHECK(client.reliable.size() == 1);
    CHECK(client.reliable[0] == Message(PlayerLeft{3, 1}));
}

void testKeepaliveAndNoAckPingPong()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient client("1.1.1.1|1000", "Bob", now);

    CHECK(connect(harness, client, now));
    harness.sent.clear();
    harness.gateway.flush(now);
    CHECK(harness.sent.empty());
    harness.gateway.flush(now + 1100ms);
    CHECK(harness.takeSentTo(client.address).size() == 1);
    const auto empty = client.connection->buildDatagram(now + 1200ms);

    harness.gateway.onDatagram(client.address, empty, now + 1200ms, 1000);
    harness.gateway.flush(now + 1210ms);
    CHECK(harness.sent.empty());
}

void testCloseAllSaysGoodbye()
{
    Harness harness;
    TimePoint now = Clock::now();
    TestClient client("1.1.1.1|1000", "Bob", now);

    CHECK(connect(harness, client, now));
    harness.sent.clear();
    harness.gateway.closeAll(DisconnectReason::SHUTDOWN, now);
    const auto bye = harness.takeSentTo(client.address);

    CHECK(bye.size() == 1);
    CHECK(PacketReader::hasRecord(bye[0], MessageType::GOODBYE));
    CHECK(harness.gateway.clientCount() == 0);
}

} // namespace

int main()
{
    testClientConnectsAndGetsAWelcome();
    testPlayersAreIntroducedToEachOther();
    testSlotsRunOut();
    testRepeatedHelloGetsTheSameWelcomeAgain();
    testNewCookieReplacesTheConnection();
    testGoodbyeRemovesTheClient();
    testStrangerDatagramsAreIgnored();
    testInvalidFloodDropsTheClient();
    testSimulationMessagesReachTheClient();
    testKeepaliveAndNoAckPingPong();
    testCloseAllSaysGoodbye();
    return TEST_RESULT();
}
