/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConformanceTests.cpp
*/

#include <random>
#include <string>
#include <vector>

#include "Connection.hpp"
#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"
#include "PacketReader.hpp"
#include "ServerHandshake.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;

constexpr std::uint32_t SESSION = 0xA1B2C3D4;

std::vector<std::string> validDatagrams()
{
    Connection connection(SESSION);
    std::vector<std::string> packets;
    const std::vector<Message> unreliable = {
        Input{258, {0x14, 0x00}},
        Snapshot{0x1A2B, 258, {{5, 1280, 2880}, {9, 10240, -256}}}, Goodbye{0}};
    const TimePoint now = Clock::now();

    connection.queueReliable(Spawn{9, 3, 0, 0, 1280, 2880, 8192, 0});
    connection.queueReliable(PlayerJoined{1, "Alice"});
    connection.queueReliable(Score{0, 99});
    packets.push_back(connection.buildDatagram(now, unreliable));
    packets.push_back(Connection(SESSION).buildDatagram(now));
    return packets;
}

void testRandomDatagramsNeverCrash()
{
    std::mt19937 random(99);
    Connection connection(SESSION);
    ServerHandshake server(CookieFactory::randomSecret());
    const TimePoint now = Clock::now();

    for (int round = 0; round < 200000; ++round) {
        std::string packet(random() % 80, '\0');

        for (char &byte : packet) {
            byte = static_cast<char>(random());
        }
        if (packet.size() >= 4 && round % 2 == 0) {
            packet.replace(0, 4, "\x52\x54\x01\x00", 4);
        }
        static_cast<void>(PacketReader::isValid(packet));
        static_cast<void>(connection.receive(packet, now));
        static_cast<void>(server.handle("1.1.1.1:1", packet, now, 1, true));
    }
    CHECK(true);
}

void testEveryTruncationIsHandled()
{
    for (const std::string &packet : validDatagrams()) {
        Connection receiver(SESSION);

        for (std::size_t size = 0; size < packet.size(); ++size) {
            const bool valid = PacketReader::isValid(packet.substr(0, size));
            const bool atHeaderEnd = size == PACKET_HEADER_SIZE;
            const bool decoded =
                receiver.receive(packet.substr(0, size), Clock::now()).status ==
                DatagramStatus::ACCEPTED;

            CHECK(!valid || atHeaderEnd || size > PACKET_HEADER_SIZE);
            CHECK(!decoded || valid);
        }
    }
}

void testEverySingleBitMutationIsHandled()
{
    for (const std::string &packet : validDatagrams()) {
        for (std::size_t bit = 0; bit < packet.size() * 8; ++bit) {
            std::string mutated = packet;
            Connection receiver(SESSION);

            mutated[bit / 8] =
                static_cast<char>(mutated[bit / 8] ^ (1U << (bit % 8)));
            const auto result = receiver.receive(mutated, Clock::now());

            CHECK(result.status != DatagramStatus::ACCEPTED ||
                  PacketReader::isValid(mutated));
        }
    }
}

void testOversizedDatagramIsRejected()
{
    std::string packet = validDatagrams()[1];

    packet.append(MAX_PACKET_SIZE, 'x');
    CHECK(!PacketReader::isValid(packet));
    CHECK(Connection(SESSION).receive(packet, Clock::now()).status ==
          DatagramStatus::INVALID);
}

void testTooManyRecordsAreRejected()
{
    std::string packet;

    PacketBuilder::writeHeader(
        packet, MessageHeader{PROTOCOL_MAGIC, PROTOCOL_VERSION, 0, 0, 0, 0, 0});
    for (std::size_t i = 0; i < MAX_RECORDS + 1; ++i) {
        MessageCodec::appendMessage(packet, Goodbye{0});
    }
    CHECK(!PacketReader::isValid(packet));
}

int main()
{
    testRandomDatagramsNeverCrash();
    testEveryTruncationIsHandled();
    testEverySingleBitMutationIsHandled();
    testOversizedDatagramIsRejected();
    testTooManyRecordsAreRejected();
    return TEST_RESULT();
}
