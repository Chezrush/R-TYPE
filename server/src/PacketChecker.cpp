/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** PacketChecker
*/

#include "PacketChecker.hpp"
#include "ServerHelpers.hpp"

namespace rtype::server {

namespace {

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
    const std::uint16_t width = ServerHelpers::readUint16BigEndian(&body[4]);
    const std::uint16_t height = ServerHelpers::readUint16BigEndian(&body[6]);

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

} /* namespace */

// clang-format off
const std::unordered_map<MessageType, PacketChecker::BodyCheck>
    PacketChecker::BODY_CHECKS = {
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

bool PacketChecker::isValid(const std::string &payload) noexcept
{
    return parsHeader(payload) && parsRecords(payload);
}

std::optional<MessageHeader>
PacketChecker::parsHeader(const std::string &payload) noexcept
{
    if (payload.size() < PACKET_HEADER_SIZE ||
        payload.size() > MAX_PACKET_SIZE) {
        return std::nullopt;
    }
    const MessageHeader header = readHeader(payload);

    if (!isHeaderValid(header)) {
        return std::nullopt;
    }
    return header;
}

MessageHeader PacketChecker::readHeader(const std::string &payload) noexcept
{
    const char *data = payload.data();
    MessageHeader header;

    header.magicNumber = ServerHelpers::readUint16BigEndian(data);
    header.version = static_cast<std::uint8_t>(data[2]);
    header.flags = static_cast<std::uint8_t>(data[3]);
    header.session = ServerHelpers::readUint32BigEndian(data + 4);
    header.seq = ServerHelpers::readUint16BigEndian(data + 8);
    header.ack = ServerHelpers::readUint16BigEndian(data + 10);
    header.ackBits = ServerHelpers::readUint32BigEndian(data + 12);
    return header;
}

bool PacketChecker::isHeaderValid(const MessageHeader &header) noexcept
{
    const bool ackValid = (header.flags & FLAG_ACK_VALID) != 0;
    const bool unknownFlags = (header.flags & ~FLAG_ACK_VALID) != 0;
    const bool ackWithoutFlag = header.ack != 0 || header.ackBits != 0;

    return header.magicNumber == PROTOCOL_MAGIC &&
           header.version == PROTOCOL_VERSION && !unknownFlags &&
           (ackValid || !ackWithoutFlag);
}

bool PacketChecker::parsRecords(const std::string &payload) noexcept
{
    std::string_view remaining(payload);
    std::size_t count = 0;

    remaining.remove_prefix(PACKET_HEADER_SIZE);
    while (!remaining.empty()) {
        const auto record = readRecord(remaining);

        if (!record || ++count > MAX_RECORDS || !isRecordValid(*record)) {
            return false;
        }
    }
    return true;
}

std::optional<PacketChecker::Record>
PacketChecker::readRecord(std::string_view &remaining) noexcept
{
    if (remaining.size() < RECORD_PREFIX_SIZE) {
        return std::nullopt;
    }
    const std::size_t length =
        ServerHelpers::readUint16BigEndian(remaining.data() + 1);

    if (length > remaining.size() - RECORD_PREFIX_SIZE) {
        return std::nullopt;
    }
    Record record{byteAt(remaining, 0),
                  remaining.substr(RECORD_PREFIX_SIZE, length)};

    remaining.remove_prefix(RECORD_PREFIX_SIZE + length);
    return record;
}

bool PacketChecker::isRecordValid(const Record &record) noexcept
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

} /* namespace rtype::server */
