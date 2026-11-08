/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MessageCodecTests.cpp
*/

#include <random>
#include <string>
#include <vector>

#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"
#include "PacketReader.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;

std::vector<Message> sampleMessages()
{
    return {Hello{0x0102030405060708ULL, "Bob"},
            Challenge{42},
            Welcome{2, 4, 60, 20, 1280, 720, 300, 99},
            Reject{static_cast<std::uint8_t>(RejectReason::SERVER_FULL)},
            Goodbye{static_cast<std::uint8_t>(DisconnectReason::QUIT)},
            Input{258, {0x14, 0x00, 0x1F}},
            Snapshot{0x1A2B, 258, {{5, 1280, 2880}, {9, 10240, -256}}},
            Spawn{9, 3, 0, 0, 1280, 2880, 8192, 0},
            Destroy{9, 1, 0},
            PlayerJoined{1, "Alice"},
            PlayerLeft{1, 1},
            Score{0, 123456},
            GameState{1, 3}};
}

void testEveryMessageRoundTrips()
{
    for (const Message &message : sampleMessages()) {
        const std::string body = MessageCodec::encodeBody(message);
        const auto decoded = MessageCodec::decodeBody(
            static_cast<std::uint8_t>(typeOf(message)), body);

        CHECK(decoded.has_value());
        CHECK(decoded && *decoded == message);
    }
}

void testEveryTruncationIsRejected()
{
    for (const Message &message : sampleMessages()) {
        const std::string body = MessageCodec::encodeBody(message);
        const auto type = static_cast<std::uint8_t>(typeOf(message));

        for (std::size_t size = 0; size < body.size(); ++size) {
            CHECK(!MessageCodec::decodeBody(type, body.substr(0, size)));
        }
        CHECK(!MessageCodec::decodeBody(type, body + 'x'));
    }
}

void testUnknownTypeIsNotDecoded()
{
    CHECK(!MessageCodec::decodeBody(0x7F, "abc"));
    CHECK(!MessageCodec::decodeBody(0xFE, "abc"));
}

void testEncodingBringsValuesInsideTheLimits()
{
    Hello hello{0, std::string(40, 'a') + "\x01"};
    Input input{0, std::vector<std::uint8_t>(20, 0xFF)};
    Snapshot snapshot;

    snapshot.entities.resize(300);
    const auto decodedHello =
        MessageCodec::decodeBody(0x01, MessageCodec::encodeBody(hello));
    const auto decodedInput =
        MessageCodec::decodeBody(0x10, MessageCodec::encodeBody(input));
    const auto decodedSnapshot =
        MessageCodec::decodeBody(0x11, MessageCodec::encodeBody(snapshot));

    CHECK(decodedHello && std::get<Hello>(*decodedHello).name.size() == 16);
    CHECK(decodedInput && std::get<Input>(*decodedInput).keys.size() == 8);
    CHECK(decodedInput && std::get<Input>(*decodedInput).keys[0] == KEYS_MASK);
    CHECK(decodedSnapshot &&
          std::get<Snapshot>(*decodedSnapshot).entities.size() == 128);
}

void testNonPrintableNameIsSanitized()
{
    const auto decoded = MessageCodec::decodeBody(
        0x01, MessageCodec::encodeBody(Hello{0, "a\nb"}));

    CHECK(decoded && std::get<Hello>(*decoded).name == "a?b");
}

void testRfcSpawnExample()
{
    std::string packet;
    const Spawn spawn{9, 3, 0, 0, 1280, 2880, 8192, 0};
    const std::string expected(
        "\x81\x00\x0F\x00\x03\x00\x09\x03\x00\x00\x05\x00\x0B\x40\x20\x00\x00"
        "\x00",
        18);

    MessageCodec::appendReliable(packet, spawn, 3);
    CHECK(packet == expected);
}

void testRecordSizeMatchesTheRecord()
{
    for (const Message &message : sampleMessages()) {
        std::string packet;

        if (isReliable(typeOf(message))) {
            MessageCodec::appendReliable(packet, message, 0);
        } else {
            MessageCodec::appendMessage(packet, message);
        }
        CHECK(packet.size() == MessageCodec::recordSize(message));
    }
}

const std::vector<std::uint8_t> KNOWN_TYPES = {0x01, 0x02, 0x03, 0x04,
                                               0x05, 0x10, 0x11};

void testDecoderAgreesWithThePacketReader()
{
    std::mt19937 random(12345);
    const std::string header(
        "\x52\x54\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);

    for (int round = 0; round < 20000; ++round) {
        std::string body(random() % 20, '\0');

        for (char &byte : body) {
            byte = static_cast<char>(random() & 0x1F ? random() : 0);
        }
        const auto type = KNOWN_TYPES[random() % KNOWN_TYPES.size()];
        std::string packet = header;

        PacketBuilder::appendRecord(packet, static_cast<MessageType>(type),
                                    body);
        const bool reader = PacketReader::isValid(packet);
        const bool codec = MessageCodec::decodeBody(type, body).has_value();

        CHECK(reader == codec);
    }
}

int main()
{
    testEveryMessageRoundTrips();
    testEveryTruncationIsRejected();
    testUnknownTypeIsNotDecoded();
    testEncodingBringsValuesInsideTheLimits();
    testNonPrintableNameIsSanitized();
    testRfcSpawnExample();
    testRecordSizeMatchesTheRecord();
    testDecoderAgreesWithThePacketReader();
    return TEST_RESULT();
}
