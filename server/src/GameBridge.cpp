/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameBridge.cpp
*/

#include "GameBridge.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "FixedPoint.hpp"
#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"

namespace rtype::server {

using protocol::Message;
using protocol::MessageCodec;

namespace {

std::string reliablePayload(const std::vector<Message> &messages)
{
    std::string packet;

    protocol::PacketBuilder::writeHeader(packet, protocol::MessageHeader{});
    for (const Message &message : messages) {
        MessageCodec::appendReliable(packet, message, 0);
    }
    return packet;
}

Message toMessage(const game::SpawnEvent &event)
{
    protocol::Spawn spawn;

    spawn.netId = event.netId;
    spawn.kind = static_cast<std::uint8_t>(event.kind);
    spawn.variant = event.variant;
    spawn.owner = event.owner;
    spawn.x = protocol::toFixed(event.position.x);
    spawn.y = protocol::toFixed(event.position.y);
    spawn.vx = protocol::toFixed(event.velocity.x);
    spawn.vy = protocol::toFixed(event.velocity.y);
    return spawn;
}

Message toMessage(const game::DestroyEvent &event)
{
    return protocol::Destroy{
        event.netId, static_cast<std::uint8_t>(event.reason), event.killer};
}

Message toMessage(const game::ScoreEvent &event)
{
    return protocol::Score{event.player, event.score};
}

Message toMessage(const game::GameStateEvent &event)
{
    return protocol::GameState{static_cast<std::uint8_t>(event.phase),
                               event.level};
}

} // namespace

GameBridge::GameBridge(const game::GameRules &rules, std::uint32_t seed,
                       std::uint8_t snapshotRate)
    : _simulation(rules, seed),
      _snapshotEvery(std::max<std::uint32_t>(
          1, rules.tickRate / std::max<std::uint8_t>(1, snapshotRate)))
{}

std::vector<OutgoingMessage> GameBridge::addClient(std::uint32_t clientId,
                                                   std::uint8_t slot)
{
    std::vector<Message> world;

    for (const game::SpawnEvent &event : _simulation.currentWorld()) {
        world.push_back(toMessage(event));
    }
    _simulation.addPlayer(slot);
    _clients[clientId].slot = slot;
    if (world.empty()) {
        return {};
    }
    return {OutgoingMessage{clientId, reliablePayload(world)}};
}

void GameBridge::removeClient(std::uint32_t clientId)
{
    const auto found = _clients.find(clientId);

    if (found == _clients.end()) {
        return;
    }
    _simulation.removePlayer(found->second.slot);
    _clients.erase(found);
}

void GameBridge::onInput(std::uint32_t clientId, const protocol::Input &input)
{
    const auto found = _clients.find(clientId);

    if (found != _clients.end()) {
        found->second.inputs.push(input);
    }
}

std::vector<OutgoingMessage>
GameBridge::broadcast(const std::vector<Message> &events) const
{
    std::vector<OutgoingMessage> out;

    if (events.empty()) {
        return out;
    }
    const std::string payload = reliablePayload(events);

    for (const auto &[clientId, client] : _clients) {
        out.push_back(OutgoingMessage{clientId, payload});
    }
    return out;
}

std::string
GameBridge::snapshotPacket(const Client &client,
                           const std::vector<game::EntityState> &states) const
{
    protocol::Snapshot snapshot;
    std::string packet;

    snapshot.tick = static_cast<std::uint32_t>(_simulation.tick());
    snapshot.inputAck = client.inputs.applied();
    for (const game::EntityState &state : states) {
        snapshot.entities.push_back({state.netId,
                                     protocol::toFixed(state.position.x),
                                     protocol::toFixed(state.position.y)});
    }
    protocol::PacketBuilder::writeHeader(packet, protocol::MessageHeader{});
    MessageCodec::appendMessage(packet, snapshot);
    return packet;
}

std::vector<OutgoingMessage> GameBridge::snapshots() const
{
    std::vector<OutgoingMessage> out;
    const auto states = _simulation.states();

    out.reserve(_clients.size());
    for (const auto &[clientId, client] : _clients) {
        out.push_back(
            OutgoingMessage{clientId, snapshotPacket(client, states)});
    }
    return out;
}

std::vector<OutgoingMessage> GameBridge::step()
{
    for (auto &[clientId, client] : _clients) {
        _simulation.setInput(client.slot, client.inputs.next());
    }
    _simulation.step();
    std::vector<Message> events;

    for (const game::GameEvent &event : _simulation.takeEvents()) {
        events.push_back(
            std::visit([](const auto &e) { return toMessage(e); }, event));
    }
    std::vector<OutgoingMessage> out = broadcast(events);

    if (_simulation.tick() % _snapshotEvery == 0) {
        auto shots = snapshots();

        out.insert(out.end(), std::make_move_iterator(shots.begin()),
                   std::make_move_iterator(shots.end()));
    }
    return out;
}

std::size_t GameBridge::clientCount() const noexcept
{
    return _clients.size();
}

const game::GameRules &GameBridge::rules() const noexcept
{
    return _simulation.rules();
}

std::uint64_t GameBridge::tick() const noexcept
{
    return _simulation.tick();
}

} /* namespace rtype::server */
