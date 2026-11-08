/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** PlayerSessions
*/

#include "PlayerSessions.hpp"

#include <algorithm>
#include <string>

#include "PacketBuilder.hpp"

namespace rtype::server {

using protocol::DisconnectReason;
using protocol::MAX_PLAYERS;
using protocol::PacketBuilder;

/**
 * @brief Builds a packet holding one player_left record.
 *
 * session, seq and rel_id stay at 0 until the reliable channel fills them.
 */
std::string makePlayerLeftPacket(std::uint8_t index, DisconnectReason reason)
{
    std::string packet;

    PacketBuilder::writeHeader(packet, protocol::MessageHeader{});
    PacketBuilder::appendPlayerLeft(packet, 0, index, reason);
    return packet;
}

PlayerSessions::PlayerSessions(std::uint8_t maxPlayers)
    : _maxPlayers(std::min(maxPlayers, MAX_PLAYERS))
{}

std::optional<std::uint8_t>
PlayerSessions::join(std::uint32_t clientId, std::optional<std::uint8_t> wanted)
{
    if (contains(clientId)) {
        return std::nullopt;
    }
    const auto slot = wanted && isFree(*wanted) ? wanted : firstFreeSlot();

    if (slot) {
        _players.emplace(clientId, *slot);
    }
    return slot;
}

std::vector<OutgoingMessage> PlayerSessions::leave(std::uint32_t clientId,
                                                   DisconnectReason reason)
{
    const auto player = _players.find(clientId);

    if (player == _players.end()) {
        return {};
    }
    const std::uint8_t index = player->second;
    _players.erase(player);
    const std::string packet = makePlayerLeftPacket(index, reason);
    std::vector<OutgoingMessage> notices;

    notices.reserve(_players.size());
    for (const auto &[otherId, otherIndex] : _players) {
        notices.push_back({otherId, packet});
    }
    return notices;
}

std::vector<std::uint32_t> PlayerSessions::clientIds() const
{
    std::vector<std::uint32_t> ids;

    ids.reserve(_players.size());
    for (const auto &[clientId, slot] : _players) {
        ids.push_back(clientId);
    }
    return ids;
}

bool PlayerSessions::contains(std::uint32_t clientId) const noexcept
{
    return _players.contains(clientId);
}

std::size_t PlayerSessions::size() const noexcept
{
    return _players.size();
}

bool PlayerSessions::isFree(std::uint8_t slot) const noexcept
{
    const auto usesSlot = [slot](const auto &player) {
        return player.second == slot;
    };

    return slot < _maxPlayers && std::ranges::none_of(_players, usesSlot);
}

std::optional<std::uint8_t> PlayerSessions::firstFreeSlot() const noexcept
{
    for (std::uint8_t slot = 0; slot < _maxPlayers; ++slot) {
        if (isFree(slot)) {
            return slot;
        }
    }
    return std::nullopt;
}

} /* namespace rtype::server */
