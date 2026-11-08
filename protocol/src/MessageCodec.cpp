/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MessageCodec.cpp
*/

#include "MessageCodec.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "PacketBuilder.hpp"

namespace rtype::protocol {

namespace {

bool isPrintableAscii(std::string_view text) noexcept
{
    return std::all_of(text.begin(), text.end(),
                       [](char c) { return c >= 0x20 && c <= 0x7E; });
}

bool isPlayerOrNone(std::uint8_t value) noexcept
{
    return value < MAX_PLAYERS || value == NO_PLAYER;
}

std::string cleanName(std::string_view name)
{
    std::string cleaned(name.substr(0, MAX_NAME_LENGTH));

    for (char &character : cleaned) {
        if (character < 0x20 || character > 0x7E) {
            character = '?';
        }
    }
    return cleaned;
}

std::optional<std::string> readName(ByteReader &reader) noexcept
{
    const std::size_t length = reader.readU8();

    if (length > MAX_NAME_LENGTH) {
        return std::nullopt;
    }
    const std::string_view name = reader.readBytes(length);

    if (!reader.finished() || !isPrintableAscii(name)) {
        return std::nullopt;
    }
    return std::string(name);
}

void encodeFields(ByteWriter &out, const Hello &m)
{
    const std::string name = cleanName(m.name);

    out.writeU64(m.cookie).writeU8(static_cast<std::uint8_t>(name.size()));
    out.writeBytes(name);
}

void encodeFields(ByteWriter &out, const Challenge &m)
{
    out.writeU64(m.cookie);
}

void encodeFields(ByteWriter &out, const Welcome &m)
{
    out.writeU8(m.playerIndex).writeU8(m.maxPlayers).writeU8(m.tickRate);
    out.writeU8(m.snapshotRate).writeU16(m.worldWidth).writeU16(m.worldHeight);
    out.writeU16(m.shipSpeed).writeU32(m.serverTick);
}

void encodeFields(ByteWriter &out, const Reject &m)
{
    out.writeU8(m.reason);
}

void encodeFields(ByteWriter &out, const Goodbye &m)
{
    out.writeU8(m.reason);
}

void encodeFields(ByteWriter &out, const Input &m)
{
    const std::size_t count = std::min(m.keys.size(), MAX_INPUTS);

    out.writeU16(m.firstId).writeU8(static_cast<std::uint8_t>(count));
    for (std::size_t i = 0; i < count; ++i) {
        out.writeU8(m.keys[i] & KEYS_MASK);
    }
}

void encodeFields(ByteWriter &out, const Snapshot &m)
{
    const std::size_t count =
        std::min(m.entities.size(), MAX_SNAPSHOT_ENTITIES);

    out.writeU32(m.tick).writeU16(m.inputAck);
    out.writeU8(static_cast<std::uint8_t>(count));
    for (std::size_t i = 0; i < count; ++i) {
        out.writeU16(m.entities[i].netId).writeI16(m.entities[i].x);
        out.writeI16(m.entities[i].y);
    }
}

void encodeFields(ByteWriter &out, const Spawn &m)
{
    out.writeU16(m.netId).writeU8(m.kind).writeU8(m.variant).writeU8(m.owner);
    out.writeI16(m.x).writeI16(m.y).writeI16(m.vx).writeI16(m.vy);
}

void encodeFields(ByteWriter &out, const Destroy &m)
{
    out.writeU16(m.netId).writeU8(m.reason).writeU8(m.killer);
}

void encodeFields(ByteWriter &out, const PlayerJoined &m)
{
    const std::string name = cleanName(m.name);

    out.writeU8(m.playerIndex).writeU8(static_cast<std::uint8_t>(name.size()));
    out.writeBytes(name);
}

void encodeFields(ByteWriter &out, const PlayerLeft &m)
{
    out.writeU8(m.playerIndex).writeU8(m.reason);
}

void encodeFields(ByteWriter &out, const Score &m)
{
    out.writeU8(m.playerIndex).writeU32(m.score);
}

void encodeFields(ByteWriter &out, const GameState &m)
{
    out.writeU8(m.phase).writeU8(m.level);
}

std::optional<Message> decodeHello(ByteReader &reader) noexcept
{
    const std::uint64_t cookie = reader.readU64();
    auto name = readName(reader);

    if (!name) {
        return std::nullopt;
    }
    return Hello{cookie, std::move(*name)};
}

std::optional<Message> decodeWelcome(ByteReader &reader) noexcept
{
    Welcome m;

    m.playerIndex = reader.readU8();
    m.maxPlayers = reader.readU8();
    m.tickRate = reader.readU8();
    m.snapshotRate = reader.readU8();
    m.worldWidth = reader.readU16();
    m.worldHeight = reader.readU16();
    m.shipSpeed = reader.readU16();
    m.serverTick = reader.readU32();
    const bool valid =
        reader.finished() && m.maxPlayers >= 1 && m.maxPlayers <= MAX_PLAYERS &&
        m.playerIndex < m.maxPlayers && m.tickRate >= 1 && m.tickRate <= 240 &&
        m.snapshotRate >= 1 && m.snapshotRate <= m.tickRate &&
        m.worldWidth >= 1 && m.worldHeight >= 1;

    return valid ? std::optional<Message>(m) : std::nullopt;
}

std::optional<Message> decodeInput(ByteReader &reader) noexcept
{
    Input m;

    m.firstId = reader.readU16();
    const std::size_t count = reader.readU8();

    if (count < 1 || count > MAX_INPUTS) {
        return std::nullopt;
    }
    for (std::size_t i = 0; i < count; ++i) {
        m.keys.push_back(reader.readU8());
    }
    const bool valid =
        reader.finished() &&
        std::all_of(m.keys.begin(), m.keys.end(), [](auto keys) {
            return (keys & static_cast<std::uint8_t>(~KEYS_MASK)) == 0;
        });

    return valid ? std::optional<Message>(std::move(m)) : std::nullopt;
}

std::optional<Message> decodeSnapshot(ByteReader &reader) noexcept
{
    Snapshot m;

    m.tick = reader.readU32();
    m.inputAck = reader.readU16();
    const std::size_t count = reader.readU8();

    if (count > MAX_SNAPSHOT_ENTITIES || reader.remaining() != count * 6) {
        return std::nullopt;
    }
    for (std::size_t i = 0; i < count; ++i) {
        EntityState state;

        state.netId = reader.readU16();
        state.x = reader.readI16();
        state.y = reader.readI16();
        m.entities.push_back(state);
    }
    return reader.finished() ? std::optional<Message>(std::move(m))
                             : std::nullopt;
}

std::optional<Message> decodeSpawn(ByteReader &reader) noexcept
{
    Spawn m;

    m.netId = reader.readU16();
    m.kind = reader.readU8();
    m.variant = reader.readU8();
    m.owner = reader.readU8();
    m.x = reader.readI16();
    m.y = reader.readI16();
    m.vx = reader.readI16();
    m.vy = reader.readI16();
    const bool valid = reader.finished() && isPlayerOrNone(m.owner);

    return valid ? std::optional<Message>(m) : std::nullopt;
}

std::optional<Message> decodeDestroy(ByteReader &reader) noexcept
{
    Destroy m;

    m.netId = reader.readU16();
    m.reason = reader.readU8();
    m.killer = reader.readU8();
    const bool valid = reader.finished() && isPlayerOrNone(m.killer);

    return valid ? std::optional<Message>(m) : std::nullopt;
}

std::optional<Message> decodePlayerJoined(ByteReader &reader) noexcept
{
    const std::uint8_t index = reader.readU8();
    auto name = readName(reader);

    if (!name || index >= MAX_PLAYERS) {
        return std::nullopt;
    }
    return PlayerJoined{index, std::move(*name)};
}

std::optional<Message> decodePlayerLeft(ByteReader &reader) noexcept
{
    PlayerLeft m;

    m.playerIndex = reader.readU8();
    m.reason = reader.readU8();
    const bool valid = reader.finished() && m.playerIndex < MAX_PLAYERS;

    return valid ? std::optional<Message>(m) : std::nullopt;
}

std::optional<Message> decodeScore(ByteReader &reader) noexcept
{
    Score m;

    m.playerIndex = reader.readU8();
    m.score = reader.readU32();
    const bool valid = reader.finished() && m.playerIndex < MAX_PLAYERS;

    return valid ? std::optional<Message>(m) : std::nullopt;
}

std::optional<Message> decodeChallenge(ByteReader &reader) noexcept
{
    const Challenge m{reader.readU64()};

    return reader.finished() ? std::optional<Message>(m) : std::nullopt;
}

std::optional<Message> decodeReject(ByteReader &reader) noexcept
{
    const Reject m{reader.readU8()};

    return reader.finished() ? std::optional<Message>(m) : std::nullopt;
}

std::optional<Message> decodeGoodbye(ByteReader &reader) noexcept
{
    const Goodbye m{reader.readU8()};

    return reader.finished() ? std::optional<Message>(m) : std::nullopt;
}

std::optional<Message> decodeGameState(ByteReader &reader) noexcept
{
    GameState m;

    m.phase = reader.readU8();
    m.level = reader.readU8();
    return reader.finished() ? std::optional<Message>(m) : std::nullopt;
}

} /* namespace */

std::string MessageCodec::encodeBody(const Message &message)
{
    std::string body;
    ByteWriter writer(body);

    std::visit([&writer](const auto &m) { encodeFields(writer, m); }, message);
    return body;
}

namespace {

using Decoder = std::optional<Message> (*)(ByteReader &) noexcept;

const std::unordered_map<MessageType, Decoder> DECODERS = {
    {MessageType::HELLO, decodeHello},
    {MessageType::CHALLENGE, decodeChallenge},
    {MessageType::WELCOME, decodeWelcome},
    {MessageType::REJECT, decodeReject},
    {MessageType::GOODBYE, decodeGoodbye},
    {MessageType::INPUT, decodeInput},
    {MessageType::SNAPSHOT, decodeSnapshot},
    {MessageType::SPAWN, decodeSpawn},
    {MessageType::DESTROY, decodeDestroy},
    {MessageType::PLAYERJOINED, decodePlayerJoined},
    {MessageType::PLAYERLEFT, decodePlayerLeft},
    {MessageType::SCORE, decodeScore},
    {MessageType::GAMESTATE, decodeGameState},
};

} // namespace

std::optional<Message> MessageCodec::decodeBody(std::uint8_t type,
                                                std::string_view body) noexcept
{
    const auto decoder = DECODERS.find(static_cast<MessageType>(type));

    if (decoder == DECODERS.end()) {
        return std::nullopt;
    }
    ByteReader reader(body);

    return decoder->second(reader);
}

void MessageCodec::appendMessage(std::string &packet, const Message &message)
{
    PacketBuilder::appendRecord(packet, typeOf(message), encodeBody(message));
}

void MessageCodec::appendReliable(std::string &packet, const Message &message,
                                  std::uint16_t relId)
{
    std::string body;

    ByteWriter(body).writeU16(relId).writeBytes(encodeBody(message));
    PacketBuilder::appendRecord(packet, typeOf(message), body);
}

std::size_t MessageCodec::recordSize(const Message &message)
{
    const std::size_t relId = isReliable(typeOf(message)) ? REL_ID_SIZE : 0;

    return RECORD_PREFIX_SIZE + relId + encodeBody(message).size();
}

} /* namespace rtype::protocol */
