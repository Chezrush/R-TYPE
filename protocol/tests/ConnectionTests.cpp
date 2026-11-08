/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConnectionTests.cpp
*/

#include <deque>
#include <random>
#include <string>
#include <vector>

#include "Connection.hpp"
#include "PacketBuilder.hpp"
#include "PacketReader.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;
using namespace std::chrono_literals;

constexpr std::uint32_t SESSION = 0xA1B2C3D4;

void testFirstDatagramHasNoAck()
{
    Connection connection(SESSION);
    const std::string packet = connection.buildDatagram(Clock::now());
    const auto header = PacketReader::parseHeader(packet);

    CHECK(header.has_value());
    CHECK(header && header->session == SESSION);
    CHECK(header && (header->flags & FLAG_ACK_VALID) == 0);
    CHECK(packet.size() == PACKET_HEADER_SIZE);
}

void testUnreliableMessagesArrive()
{
    Connection sender(SESSION);
    Connection receiver(SESSION);
    const std::vector<Message> messages = {Input{1, {0x04}}, Goodbye{0}};
    const auto result = receiver.receive(
        sender.buildDatagram(Clock::now(), messages), Clock::now());

    CHECK(result.status == DatagramStatus::ACCEPTED);
    CHECK(result.hadRecords);
    CHECK(result.unreliable.size() == 2);
    CHECK(result.unreliable[0] == Message(Input{1, {0x04}}));
}

void testEmptyDatagramIsAnAcknowledgement()
{
    Connection sender(SESSION);
    Connection receiver(SESSION);
    const auto result =
        receiver.receive(sender.buildDatagram(Clock::now()), Clock::now());

    CHECK(result.status == DatagramStatus::ACCEPTED);
    CHECK(!result.hadRecords);
}

void testWrongSessionIsRejected()
{
    Connection sender(SESSION);
    Connection receiver(SESSION + 1);
    const auto result =
        receiver.receive(sender.buildDatagram(Clock::now()), Clock::now());

    CHECK(result.status == DatagramStatus::WRONG_SESSION);
}

void testDuplicateDatagramIsDropped()
{
    Connection sender(SESSION);
    Connection receiver(SESSION);
    const std::vector<Message> messages = {Goodbye{0}};
    const std::string packet = sender.buildDatagram(Clock::now(), messages);

    CHECK(receiver.receive(packet, Clock::now()).status ==
          DatagramStatus::ACCEPTED);
    const auto again = receiver.receive(packet, Clock::now());

    CHECK(again.status == DatagramStatus::DUPLICATE);
    CHECK(again.unreliable.empty());
}

void testInvalidDatagramChangesNothing()
{
    Connection sender(SESSION);
    Connection receiver(SESSION);
    std::string packet = sender.buildDatagram(Clock::now(), {});

    packet += std::string("\x10\x00\x09", 3);
    CHECK(receiver.receive(packet, Clock::now()).status ==
          DatagramStatus::INVALID);
    CHECK(receiver.receive("garbage", Clock::now()).status ==
          DatagramStatus::INVALID);
    CHECK(receiver.lastReceived() == TimePoint{});
}

void testReliableMessageIsDeliveredOnce()
{
    Connection sender(SESSION);
    Connection receiver(SESSION);
    const TimePoint now = Clock::now();

    CHECK(sender.queueReliable(Score{1, 50}));
    const std::string packet = sender.buildDatagram(now);
    const auto first = receiver.receive(packet, now);
    const auto retransmit =
        receiver.receive(sender.buildDatagram(now + 1s), now + 1s);

    CHECK(first.reliable.size() == 1);
    CHECK(first.reliable[0] == Message(Score{1, 50}));
    CHECK(retransmit.reliable.empty());
}

void testUnreliableTypeCannotBeQueuedAsReliable()
{
    Connection connection(SESSION);

    CHECK(!connection.queueReliable(Goodbye{0}));
}

void testReliableQueueOverflowIsReported()
{
    Connection connection(SESSION);
    bool refused = false;

    for (std::size_t i = 0; i <= MAX_RELIABLE_QUEUE; ++i) {
        refused = !connection.queueReliable(Score{0, 1}) || refused;
    }
    CHECK(refused);
}

void testAcknowledgementClearsTheQueueAndMeasuresRtt()
{
    Connection server(SESSION);
    Connection client(SESSION);
    const TimePoint start = Clock::now();

    server.queueReliable(GameState{1, 1});
    const auto delivered =
        client.receive(server.buildDatagram(start), start + 20ms);
    const auto ackBack =
        server.receive(client.buildDatagram(start + 20ms), start + 40ms);

    CHECK(delivered.reliable.size() == 1);
    CHECK(ackBack.status == DatagramStatus::ACCEPTED);
    CHECK(server.pendingReliable() == 0);
    CHECK(server.rtt().hasSample());
    CHECK(server.rtt().smoothed() == 40ms);
}

void testReliableOutOfWindowDropsTheWholeDatagram()
{
    Connection receiver(SESSION);
    std::string packet;
    const MessageHeader header{
        PROTOCOL_MAGIC, PROTOCOL_VERSION, 0, SESSION, 0, 0, 0};

    PacketBuilder::writeHeader(packet, header);
    PacketBuilder::appendRecord(packet, MessageType::SCORE,
                                std::string("\x00\x00\x00\x00\x00\x00\x00", 7));
    PacketBuilder::appendRecord(packet, MessageType::SCORE,
                                std::string("\x10\x00\x00\x00\x00\x00\x00", 7));
    const auto result = receiver.receive(packet, Clock::now());

    CHECK(result.status == DatagramStatus::RELIABLE_OUT_OF_WINDOW);
    CHECK(result.reliable.empty());
}

void testUnknownReliableTypeKeepsTheOrder()
{
    Connection receiver(SESSION);
    std::string packet;
    const MessageHeader header{
        PROTOCOL_MAGIC, PROTOCOL_VERSION, 0, SESSION, 0, 0, 0};

    PacketBuilder::writeHeader(packet, header);
    PacketBuilder::appendRecord(packet, static_cast<MessageType>(0xA0),
                                std::string("\x00\x00zz", 4));
    PacketBuilder::appendRecord(packet, MessageType::GAMESTATE,
                                std::string("\x00\x01\x01\x02", 4));
    const auto result = receiver.receive(packet, Clock::now());

    CHECK(result.status == DatagramStatus::ACCEPTED);
    CHECK(result.reliable.size() == 1);
}

void testFullDatagramStaysUnderTheLimit()
{
    Connection sender(SESSION);
    const TimePoint now = Clock::now();
    Snapshot big;

    big.entities.resize(128);
    for (int i = 0; i < 100; ++i) {
        sender.queueReliable(Spawn{});
    }
    const std::vector<Message> messages(3, big);
    const std::string packet = sender.buildDatagram(now, messages);

    CHECK(packet.size() <= MAX_PACKET_SIZE);
    CHECK(PacketReader::isValid(packet));
}

struct Link {
    std::mt19937 random{7};
    int lossPercent = 30;
    int duplicatePercent = 10;
    std::deque<std::pair<TimePoint, std::string>> flight;

    void send(const std::string &packet, TimePoint now)
    {
        if (static_cast<int>(random() % 100) < lossPercent) {
            return;
        }
        const int copies =
            static_cast<int>(random() % 100) < duplicatePercent ? 2 : 1;

        for (int i = 0; i < copies; ++i) {
            const auto jitter = std::chrono::milliseconds(random() % 60);

            flight.emplace_back(now + 30ms + jitter, packet);
        }
    }

    std::vector<std::string> arrived(TimePoint now)
    {
        std::vector<std::string> out;

        for (auto it = flight.begin(); it != flight.end();) {
            if (it->first <= now) {
                out.push_back(std::move(it->second));
                it = flight.erase(it);
            } else {
                ++it;
            }
        }
        return out;
    }
};

void testBothDirectionsSurviveALossyReorderingLink()
{
    Connection server(SESSION);
    Connection client(SESSION);
    Link toClient;
    Link toServer;
    std::vector<Message> clientGot;
    std::vector<Message> serverGot;
    TimePoint now = Clock::now();
    constexpr int TOTAL = 500;

    for (int step = 0; step < 4000; ++step, now += 10ms) {
        if (step < TOTAL) {
            server.queueReliable(Score{0, static_cast<std::uint32_t>(step)});
            client.queueReliable(Score{1, static_cast<std::uint32_t>(step)});
        }
        toClient.send(server.buildDatagram(now), now);
        toServer.send(client.buildDatagram(now), now);
        for (const auto &packet : toClient.arrived(now)) {
            auto r = client.receive(packet, now);
            clientGot.insert(clientGot.end(), r.reliable.begin(),
                             r.reliable.end());
        }
        for (const auto &packet : toServer.arrived(now)) {
            auto r = server.receive(packet, now);
            serverGot.insert(serverGot.end(), r.reliable.begin(),
                             r.reliable.end());
        }
    }
    CHECK(clientGot.size() == TOTAL);
    CHECK(serverGot.size() == TOTAL);
    for (std::size_t i = 0; i < clientGot.size() && i < serverGot.size(); ++i) {
        CHECK(std::get<Score>(clientGot[i]).score == i);
        CHECK(std::get<Score>(serverGot[i]).score == i);
    }
}

int main()
{
    testFirstDatagramHasNoAck();
    testUnreliableMessagesArrive();
    testEmptyDatagramIsAnAcknowledgement();
    testWrongSessionIsRejected();
    testDuplicateDatagramIsDropped();
    testInvalidDatagramChangesNothing();
    testReliableMessageIsDeliveredOnce();
    testUnreliableTypeCannotBeQueuedAsReliable();
    testReliableQueueOverflowIsReported();
    testAcknowledgementClearsTheQueueAndMeasuresRtt();
    testReliableOutOfWindowDropsTheWholeDatagram();
    testUnknownReliableTypeKeepsTheOrder();
    testFullDatagramStaysUnderTheLimit();
    testBothDirectionsSurviveALossyReorderingLink();
    return TEST_RESULT();
}
