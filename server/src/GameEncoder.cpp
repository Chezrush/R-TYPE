/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameEncoder.cpp
*/

#include "GameEncoder.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <variant>

#include "FixedPoint.hpp"
#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"

namespace rtype::server {

using game::EntityKind;
using protocol::Message;
using protocol::MessageCodec;
using protocol::toFixed;

namespace {

template <typename Enum>
std::uint8_t wireValue(Enum value) noexcept
{
    return static_cast<std::uint8_t>(value);
}

Message messageOf(const game::SpawnEvent &event)
{
    return protocol::Spawn{event.netId,
                           wireValue(event.kind),
                           event.variant,
                           event.owner,
                           toFixed(event.position.x),
                           toFixed(event.position.y),
                           toFixed(event.velocity.x),
                           toFixed(event.velocity.y)};
}

Message messageOf(const game::DestroyEvent &event)
{
    return protocol::Destroy{event.netId, wireValue(event.reason),
                             event.killer};
}

Message messageOf(const game::ScoreEvent &event)
{
    return protocol::Score{event.player, event.score};
}

Message messageOf(const game::GameStateEvent &event)
{
    return protocol::GameState{wireValue(event.phase), event.level};
}

std::string emptyPacket()
{
    std::string packet;

    protocol::PacketBuilder::writeHeader(packet, protocol::MessageHeader{});
    return packet;
}

int priorityOf(EntityKind kind) noexcept
{
    switch (kind) {
        case EntityKind::PLAYER_SHIP:
            return 0;
        case EntityKind::BYDOS:
            return 1;
        default:
            return 2;
    }
}

bool isMissile(EntityKind kind) noexcept
{
    return kind == EntityKind::PLAYER_MISSILE ||
           kind == EntityKind::BYDOS_MISSILE;
}

} // namespace

Message GameEncoder::toMessage(const game::GameEvent &event)
{
    return std::visit(
        [](const auto &alternative) { return messageOf(alternative); }, event);
}

std::vector<std::string>
GameEncoder::encodeEvents(std::span<const game::GameEvent> events)
{
    std::vector<std::string> packets;
    std::size_t records = 0;

    for (const game::GameEvent &event : events) {
        const Message message = toMessage(event);
        const bool full =
            packets.empty() || records == protocol::MAX_RECORDS ||
            packets.back().size() + MessageCodec::recordSize(message) >
                protocol::MAX_PACKET_SIZE;

        if (full) {
            packets.push_back(emptyPacket());
            records = 0;
        }
        MessageCodec::appendReliable(packets.back(), message, 0);
        ++records;
    }
    return packets;
}

std::uint64_t GameEncoder::snapshotInterval(std::uint32_t tickRate,
                                            std::uint32_t snapshotRate) noexcept
{
    return std::max<std::uint64_t>(1, tickRate / std::max(snapshotRate, 1U));
}

protocol::Snapshot
GameEncoder::makeSnapshot(std::uint64_t tick, std::uint64_t index,
                          std::span<const game::EntityState> states)
{
    const bool withMissiles = index % MISSILE_PERIOD == 0;
    std::vector<game::EntityState> listed;
    protocol::Snapshot snapshot;

    std::ranges::copy_if(states, std::back_inserter(listed),
                         [withMissiles](const game::EntityState &state) {
                             return withMissiles || !isMissile(state.kind);
                         });
    std::ranges::stable_sort(listed, {}, [](const game::EntityState &state) {
        return priorityOf(state.kind);
    });
    listed.resize(std::min(listed.size(), protocol::MAX_SNAPSHOT_ENTITIES));
    snapshot.tick = static_cast<std::uint32_t>(tick);
    for (const game::EntityState &state : listed) {
        snapshot.entities.push_back({state.netId, toFixed(state.position.x),
                                     toFixed(state.position.y)});
    }
    return snapshot;
}

std::string GameEncoder::encodeSnapshot(const protocol::Snapshot &snapshot)
{
    std::string packet = emptyPacket();

    MessageCodec::appendMessage(packet, snapshot);
    return packet;
}

} /* namespace rtype::server */
