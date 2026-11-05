/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** PacketBuilderTests.cpp
*/

#include <string>
#include <string_view>
#include <vector>

#include "Protocol/ByteOrder.hpp"
#include "Protocol/PacketBuilder.hpp"
#include "Protocol/PacketReader.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;

std::vector<Record> readAll(std::string_view packet)
{
    std::string_view remaining = PacketReader::recordsOf(packet);
    std::vector<Record> records;

    while (const auto record = PacketReader::readRecord(remaining)) {
        records.push_back(*record);
    }
    return records;
}

void testByteOrderRoundTrip()
{
    std::string bytes;

    writeUint16BigEndian(bytes, 0x1234);
    writeUint32BigEndian(bytes, 0xA1B2C3D4);
    CHECK(bytes == std::string("\x12\x34\xA1\xB2\xC3\xD4"));
    CHECK(readUint16BigEndian(bytes.data()) == 0x1234);
    CHECK(readUint32BigEndian(bytes.data() + 2) == 0xA1B2C3D4);
}

void testHeaderRoundTrip()
{
    MessageHeader header;
    std::string packet;

    header.flags = FLAG_ACK_VALID;
    header.session = 0xA1B2C3D4;
    header.seq = 7;
    header.ack = 0x10;
    header.ackBits = 7;
    PacketBuilder::writeHeader(packet, header);
    CHECK(packet.size() == PACKET_HEADER_SIZE);
    CHECK(PacketReader::isValid(packet));
    CHECK(PacketReader::parseHeader(packet) == header);
}

void testDocumentationExample()
{
    const std::string expected("\x52\x54\x01\x00\0\0\0\0\0\0\0\0\0\0\0\0"
                               "\x01\x00\x0C\0\0\0\0\0\0\0\0\x03"
                               "Bob",
                               31);
    std::string packet;

    PacketBuilder::writeHeader(packet, MessageHeader{});
    PacketBuilder::appendHello(packet, 0, "Bob");
    CHECK(packet == expected);
}

void testRecordsRoundTrip()
{
    std::string packet;

    PacketBuilder::writeHeader(packet, MessageHeader{});
    PacketBuilder::appendHello(packet, 0x0102030405060708, "Alice");
    PacketBuilder::appendGoodbye(packet, DisconnectReason::TIMEOUT);
    PacketBuilder::appendPlayerLeft(packet, 3, 2, DisconnectReason::QUIT);
    CHECK(PacketReader::isValid(packet));

    const std::vector<Record> expected = {
        {0x01, std::string_view("\1\2\3\4\5\6\7\x08\x05"
                                "Alice",
                                14)},
        {0x05, std::string_view("\x01", 1)},
        {0x84, std::string_view("\0\x03\x02\0", 4)},
    };

    CHECK(readAll(packet) == expected);
    CHECK(PacketReader::hasRecord(packet, MessageType::PLAYERLEFT));
    CHECK(!PacketReader::hasRecord(packet, MessageType::INPUT));
}

int main()
{
    testByteOrderRoundTrip();
    testHeaderRoundTrip();
    testDocumentationExample();
    testRecordsRoundTrip();

    return TEST_RESULT();
}
