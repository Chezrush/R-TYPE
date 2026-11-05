/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** PlayerSessions
*/

#include "PlayerSessions.hpp"

#include <array>
#include <string>

#include "Protocol/PacketBuilder.hpp"

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

bool PlayerSessions::join(std::uint32_t clientId)
{
    const auto index = freeIndex();

    if (contains(clientId) || !index) {
        return false;
    }
    _players.emplace(clientId, *index);
    return true;
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

bool PlayerSessions::contains(std::uint32_t clientId) const noexcept
{
    return _players.contains(clientId);
}

std::size_t PlayerSessions::size() const noexcept
{
    return _players.size();
}

std::optional<std::uint8_t> PlayerSessions::freeIndex() const noexcept
{
    std::array<bool, MAX_PLAYERS> taken{};

    for (const auto &[clientId, index] : _players) {
        taken[index] = true;
    }
    for (std::uint8_t index = 0; index < MAX_PLAYERS; ++index) {
        if (!taken[index]) {
            return index;
        }
    }
    return std::nullopt;
}

} /* namespace rtype::server */
