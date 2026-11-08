/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** PacketReader.cpp
*/

#include "PacketReader.hpp"

#include <unordered_map>

#include "ByteOrder.hpp"

namespace rtype::protocol {

using BodyCheck = bool (*)(std::string_view body);

/**
 * @brief Returns the byte at offset as an unsigned value.
 */
std::uint8_t byteAt(std::string_view data, std::size_t offset) noexcept
{
    return static_cast<std::uint8_t>(data[offset]);
}

/**
 * @brief Returns true if value is a valid player index.
 */
bool isPlayerIndex(std::uint8_t value) noexcept
{
    return value < MAX_PLAYERS;
}

/**
 * @brief Returns true if value is a player index or NO_PLAYER.
 */
bool isPlayerOrNone(std::uint8_t value) noexcept
{
    return isPlayerIndex(value) || value == NO_PLAYER;
}

/**
 * @brief Returns true if every character is printable ASCII.
 */
bool isPrintable(std::string_view text) noexcept
{
    for (const char character : text) {
        if (character < 0x20 || character > 0x7E) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Checks the length and the characters of the name at nameOffset.
 */
bool checkName(std::string_view body, std::size_t nameOffset) noexcept
{
    if (body.size() <= nameOffset) {
        return false;
    }
    const std::size_t length = byteAt(body, nameOffset);
    const std::string_view name = body.substr(nameOffset + 1);

    return length <= MAX_NAME_LENGTH && name.size() == length &&
           isPrintable(name);
}

/**
 * @brief Checks the size and the fields of a welcome body.
 */
bool checkWelcome(std::string_view body) noexcept
{
    if (body.size() != 14) {
        return false;
    }
    const std::uint8_t playerIndex = byteAt(body, 0);
    const std::uint8_t maxPlayers = byteAt(body, 1);
    const std::uint8_t tickRate = byteAt(body, 2);
    const std::uint8_t snapshotRate = byteAt(body, 3);
    const std::uint16_t width = readUint16BigEndian(&body[4]);
    const std::uint16_t height = readUint16BigEndian(&body[6]);

    return maxPlayers >= 1 && maxPlayers <= MAX_PLAYERS &&
           playerIndex < maxPlayers && tickRate >= 1 && tickRate <= 240 &&
           snapshotRate >= 1 && snapshotRate <= tickRate && width >= 1 &&
           height >= 1;
}

/**
 * @brief Checks the size, the count and the keys of an input body.
 */
bool checkInput(std::string_view body) noexcept
{
    if (body.size() < 3) {
        return false;
    }
    const std::size_t count = byteAt(body, 2);

    if (count < 1 || count > MAX_INPUTS || body.size() != 3 + count) {
        return false;
    }
    for (const char keys : body.substr(3)) {
        if ((static_cast<std::uint8_t>(keys) & ~KEYS_MASK) != 0) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Checks the size and the entity count of a snapshot body.
 */
bool checkSnapshot(std::string_view body) noexcept
{
    if (body.size() < 7) {
        return false;
    }
    const std::size_t count = byteAt(body, 6);

    return count <= MAX_SNAPSHOT_ENTITIES && body.size() == 7 + 6 * count;
}

// clang-format off
const std::unordered_map<MessageType, BodyCheck> BODY_CHECKS = {
    {MessageType::HELLO, [](std::string_view body) {
        return checkName(body, 8);
    }},
    {MessageType::CHALLENGE, [](std::string_view body) {
        return body.size() == 8;
    }},
    {MessageType::WELCOME, checkWelcome},
    {MessageType::REJECT, [](std::string_view body) {
        return body.size() == 1;
    }},
    {MessageType::GOODBYE, [](std::string_view body) {
        return body.size() == 1;
    }},
    {MessageType::INPUT, checkInput},
    {MessageType::SNAPSHOT, checkSnapshot},
    {MessageType::SPAWN, [](std::string_view body) {
        return body.size() == 13 && isPlayerOrNone(byteAt(body, 4));
    }},
    {MessageType::DESTROY, [](std::string_view body) {
        return body.size() == 4 && isPlayerOrNone(byteAt(body, 3));
    }},
    {MessageType::PLAYERJOINED, [](std::string_view body) {
        return !body.empty() && isPlayerIndex(byteAt(body, 0)) &&
               checkName(body, 1);
    }},
    {MessageType::PLAYERLEFT, [](std::string_view body) {
        return body.size() == 2 && isPlayerIndex(byteAt(body, 0));
    }},
    {MessageType::SCORE, [](std::string_view body) {
        return body.size() == 5 && isPlayerIndex(byteAt(body, 0));
    }},
    {MessageType::GAMESTATE, [](std::string_view body) {
        return body.size() == 2;
    }},
};
// clang-format on

bool PacketReader::isValid(std::string_view packet) noexcept
{
    return parseHeader(packet) && areRecordsValid(packet);
}

std::optional<MessageHeader>
PacketReader::parseHeader(std::string_view packet) noexcept
{
    if (packet.size() < PACKET_HEADER_SIZE || packet.size() > MAX_PACKET_SIZE) {
        return std::nullopt;
    }
    const MessageHeader header = readHeader(packet);

    if (!isHeaderValid(header)) {
        return std::nullopt;
    }
    return header;
}

std::optional<MessageHeader>
PacketReader::peekHeader(std::string_view packet) noexcept
{
    if (packet.size() < PACKET_HEADER_SIZE || packet.size() > MAX_PACKET_SIZE) {
        return std::nullopt;
    }
    return readHeader(packet);
}

bool PacketReader::hasRecord(std::string_view packet, MessageType type) noexcept
{
    std::string_view remaining = recordsOf(packet);

    while (const auto record = readRecord(remaining)) {
        if (record->type == static_cast<std::uint8_t>(type)) {
            return true;
        }
    }
    return false;
}

std::string_view PacketReader::recordsOf(std::string_view packet) noexcept
{
    if (packet.size() < PACKET_HEADER_SIZE) {
        return {};
    }
    return packet.substr(PACKET_HEADER_SIZE);
}

std::optional<Record>
PacketReader::readRecord(std::string_view &remaining) noexcept
{
    if (remaining.size() < RECORD_PREFIX_SIZE) {
        return std::nullopt;
    }
    const std::size_t length = readUint16BigEndian(remaining.data() + 1);

    if (length > remaining.size() - RECORD_PREFIX_SIZE) {
        return std::nullopt;
    }
    const Record record{byteAt(remaining, 0),
                        remaining.substr(RECORD_PREFIX_SIZE, length)};

    remaining.remove_prefix(RECORD_PREFIX_SIZE + length);
    return record;
}

MessageHeader PacketReader::readHeader(std::string_view packet) noexcept
{
    const char *data = packet.data();
    MessageHeader header;

    header.magicNumber = readUint16BigEndian(data);
    header.version = byteAt(packet, 2);
    header.flags = byteAt(packet, 3);
    header.session = readUint32BigEndian(data + 4);
    header.seq = readUint16BigEndian(data + 8);
    header.ack = readUint16BigEndian(data + 10);
    header.ackBits = readUint32BigEndian(data + 12);
    return header;
}

bool PacketReader::isHeaderValid(const MessageHeader &header) noexcept
{
    const bool ackValid = (header.flags & FLAG_ACK_VALID) != 0;
    const bool unknownFlags = (header.flags & ~FLAG_ACK_VALID) != 0;
    const bool ackWithoutFlag = header.ack != 0 || header.ackBits != 0;

    return header.magicNumber == PROTOCOL_MAGIC &&
           header.version == PROTOCOL_VERSION && !unknownFlags &&
           (ackValid || !ackWithoutFlag);
}

bool PacketReader::areRecordsValid(std::string_view packet) noexcept
{
    std::string_view remaining = recordsOf(packet);
    std::size_t count = 0;

    while (!remaining.empty()) {
        const auto record = readRecord(remaining);

        if (!record || ++count > MAX_RECORDS || !isRecordValid(*record)) {
            return false;
        }
    }
    return true;
}

bool PacketReader::isRecordValid(const Record &record) noexcept
{
    const bool reliable = (record.type & RELIABLE_BIT) != 0;

    if (record.type == 0 || (reliable && record.body.size() < REL_ID_SIZE)) {
        return false;
    }
    const std::string_view body =
        reliable ? record.body.substr(REL_ID_SIZE) : record.body;
    const auto check = BODY_CHECKS.find(static_cast<MessageType>(record.type));

    return check == BODY_CHECKS.end() || check->second(body);
}

} /* namespace rtype::protocol */
