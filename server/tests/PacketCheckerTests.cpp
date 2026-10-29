/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** PacketValidationTests.cpp
*/

#include <initializer_list>
#include <string>
#include <vector>

#include "Server.hpp"
#include "TestSupport.hpp"

using rtype::server::IncomingMessage;
using rtype::server::Server;
using rtype::server::ServerConfig;

namespace {

using Bytes = std::vector<std::uint8_t>;

ServerConfig testConfig()
{
    ServerConfig config;

    config.port = 0;
    return config;
}

/**
 * @brief Header with no session and no acknowledgement.
 */
Bytes emptyHeader()
{
    return {0x52, 0x54, 0x01, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

Bytes packet(std::initializer_list<std::uint8_t> records)
{
    Bytes bytes = emptyHeader();

    bytes.insert(bytes.end(), records);
    return bytes;
}

bool accepts(Server &server, const Bytes &bytes)
{
    return server.receivePacket(
        IncomingMessage{1, std::string(bytes.begin(), bytes.end())});
}

// Examples of section 13 of docs/protocol/rtype-protocol.md.
const Bytes HELLO_EXAMPLE = {0x52, 0x54, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x01, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x03, 0x42, 0x6F, 0x62};

const Bytes INPUT_EXAMPLE = {0x52, 0x54, 0x01, 0x01, 0xA1, 0xB2, 0xC3, 0xD4,
                             0x00, 0x07, 0x00, 0x10, 0x00, 0x00, 0x00, 0x07,
                             0x10, 0x00, 0x04, 0x01, 0x02, 0x01, 0x14};

const Bytes SNAPSHOT_EXAMPLE = {0x52, 0x54, 0x01, 0x01, 0xA1, 0xB2, 0xC3, 0xD4,
                                0x00, 0x20, 0x00, 0x07, 0x00, 0x00, 0x00, 0x01,
                                0x11, 0x00, 0x13, 0x00, 0x00, 0x1A, 0x2B, 0x01,
                                0x02, 0x02, 0x00, 0x05, 0x05, 0x00, 0x0B, 0x40,
                                0x00, 0x09, 0x28, 0x00, 0xFF, 0x00};

const Bytes SPAWN_EXAMPLE =
    packet({0x81, 0x00, 0x0F, 0x00, 0x03, 0x00, 0x09, 0x03, 0x00, 0x00, 0x05,
            0x00, 0x0B, 0x40, 0x20, 0x00, 0x00, 0x00});

} // namespace

void testValidPackets()
{
    Server server(testConfig());

    CHECK(accepts(server, HELLO_EXAMPLE));
    CHECK(accepts(server, INPUT_EXAMPLE));
    CHECK(accepts(server, SNAPSHOT_EXAMPLE));
    CHECK(accepts(server, SPAWN_EXAMPLE));
    // A packet with no record is a keepalive.
    CHECK(accepts(server, emptyHeader()));
    CHECK(server.rejectedPackets() == 0);
}

void testTruncatedPackets()
{
    Server server(testConfig());

    for (const Bytes *example :
         {&HELLO_EXAMPLE, &INPUT_EXAMPLE, &SNAPSHOT_EXAMPLE}) {
        for (std::size_t size = 0; size < example->size(); ++size) {
            // Cutting right after the header leaves a valid keepalive.
            if (size == rtype::server::PACKET_HEADER_SIZE) {
                continue;
            }
            const Bytes truncated(example->begin(),
                                  example->begin() +
                                      static_cast<std::ptrdiff_t>(size));
            CHECK(!accepts(server, truncated));
        }
    }
}

void testBitMutationsDoNotCrash()
{
    Server server(testConfig());

    for (const Bytes *example :
         {&HELLO_EXAMPLE, &INPUT_EXAMPLE, &SNAPSHOT_EXAMPLE}) {
        for (std::size_t bit = 0; bit < example->size() * 8; ++bit) {
            Bytes mutated = *example;
            mutated[bit / 8] ^= static_cast<std::uint8_t>(1U << (bit % 8));
            accepts(server, mutated);
        }
    }
}

void testInvalidHeaders()
{
    Server server(testConfig());
    Bytes bytes = emptyHeader();

    CHECK(!accepts(server, Bytes(15, 0)));
    CHECK(!accepts(server, Bytes(1201, 0)));
    bytes[0] = 0x00;
    CHECK(!accepts(server, bytes));
    bytes = emptyHeader();
    bytes[2] = 2;
    CHECK(!accepts(server, bytes));
    bytes = emptyHeader();
    bytes[3] = 0x02;
    CHECK(!accepts(server, bytes));
    // ack set without the ACK_VALID flag
    bytes = emptyHeader();
    bytes[11] = 0x01;
    CHECK(!accepts(server, bytes));
}

void testInvalidRecordSizes()
{
    Server server(testConfig());

    // record prefix cut, then a length running past the packet
    CHECK(!accepts(server, packet({0x05, 0x00})));
    CHECK(!accepts(server, packet({0x05, 0x00, 0x02, 0x00})));
    // type 0x00 and a reliable record without its rel_id
    CHECK(!accepts(server, packet({0x00, 0x00, 0x00})));
    CHECK(!accepts(server, packet({0x86, 0x00, 0x01, 0x00})));
    // goodbye with a trailing byte
    CHECK(!accepts(server, packet({0x05, 0x00, 0x02, 0x00, 0x00})));
    // input announcing 2 samples but carrying 1
    CHECK(!accepts(server, packet({0x10, 0x00, 0x04, 0x00, 0x01, 0x02, 0x00})));
    // snapshot announcing 1 entity with no entity
    CHECK(!accepts(server, packet({0x11, 0x00, 0x07, 0, 0, 0, 0, 0, 0, 0x01})));

    Bytes records = emptyHeader();
    for (int i = 0; i < 64; ++i) {
        records.insert(records.end(), {0x20, 0x00, 0x00});
    }
    CHECK(accepts(server, records));
    records.insert(records.end(), {0x20, 0x00, 0x00});
    CHECK(!accepts(server, records));
}

void testInvalidFieldValues()
{
    Server server(testConfig());

    // input count 0, then 9
    CHECK(!accepts(server, packet({0x10, 0x00, 0x03, 0x00, 0x01, 0x00})));
    CHECK(!accepts(server, packet({0x10, 0x00, 0x0C, 0x00, 0x01, 0x09, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0})));
    // input keys with a reserved bit
    CHECK(!accepts(server, packet({0x10, 0x00, 0x04, 0x00, 0x01, 0x01, 0x20})));
    // hello name longer than 16 bytes, then with a non printable character
    CHECK(!accepts(server,
                   packet({0x01, 0x00, 0x09, 0, 0, 0, 0, 0, 0, 0, 0, 0x11})));
    CHECK(!accepts(server, packet({0x01, 0x00, 0x0A, 0, 0, 0, 0, 0, 0, 0, 0,
                                   0x01, 0x0A})));
    // player_left with player_index 16
    CHECK(!accepts(server, packet({0x84, 0x00, 0x04, 0x00, 0x00, 0x10, 0x00})));
    // welcome with snapshot_rate above tick_rate
    CHECK(
        !accepts(server, packet({0x03, 0x00, 0x0E, 0x00, 0x04, 0x3C, 0x3D, 0x05,
                                 0x00, 0x02, 0xD0, 0x01, 0x2C, 0, 0, 0, 0})));
}

void testUnknownValuesAreAccepted()
{
    Server server(testConfig());

    // unknown unreliable and reliable record types are skipped
    CHECK(accepts(server, packet({0x20, 0x00, 0x02, 0xAA, 0xBB, 0xA0, 0x00,
                                  0x02, 0x00, 0x09})));
    // open enumerations: goodbye and destroy with an unknown reason
    CHECK(accepts(server, packet({0x05, 0x00, 0x01, 0xFE})));
    CHECK(accepts(server, packet({0x82, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01,
                                  0x7F, 0xFF})));
}

void testValidPacketAfterInvalidOne()
{
    Server server(testConfig());

    CHECK(!accepts(server, Bytes(3, 0)));
    CHECK(accepts(server, INPUT_EXAMPLE));
    CHECK(server.rejectedPackets() == 1);
}

int main()
{
    testValidPackets();
    testTruncatedPackets();
    testBitMutationsDoNotCrash();
    testInvalidHeaders();
    testInvalidRecordSizes();
    testInvalidFieldValues();
    testUnknownValuesAreAccepted();
    testValidPacketAfterInvalidOne();

    return TEST_RESULT();
}
