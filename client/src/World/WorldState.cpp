/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** WorldState.cpp
*/

#include "Client/World/WorldState.hpp"

#include <algorithm>
#include <variant>

#include "FixedPoint.hpp"
#include "Rtype/ShipMovement.hpp"
#include "SerialNumber.hpp"

namespace client::world {

using engine::core::Vec2;
namespace proto = rtype::protocol;

namespace {

constexpr std::size_t MAX_SAMPLES = 8;

Vec2 pixels(std::int16_t x, std::int16_t y)
{
    return {proto::fromFixed(x), proto::fromFixed(y)};
}

} // namespace

void WorldState::configure(const proto::Welcome &welcome)
{
    _rules.tickRate = welcome.tickRate;
    _rules.worldWidth = static_cast<float>(welcome.worldWidth);
    _rules.worldHeight = static_cast<float>(welcome.worldHeight);
    _rules.shipSpeed = static_cast<float>(welcome.shipSpeed);
    _rules.maxPlayers = welcome.maxPlayers;
    _localSlot = welcome.playerIndex;
    _configured = true;
}

bool WorldState::isLocalShip(const Entity &entity) const noexcept
{
    return entity.look.kind ==
               static_cast<std::uint8_t>(proto::EntityKind::PLAYER_SHIP) &&
           entity.look.owner == _localSlot;
}

void WorldState::applySpawn(const proto::Spawn &spawn, double now)
{
    Entity &entity = _entities[spawn.netId];
    const Vec2 position = pixels(spawn.x, spawn.y);

    entity.look = {spawn.netId, spawn.kind, spawn.variant, spawn.owner,
                   position};
    entity.velocity = pixels(spawn.vx, spawn.vy);
    entity.updatedAt = now;
    entity.samples.assign(1, Sample{now, position});
    if (_configured && isLocalShip(entity)) {
        _predicted = position;
        _hasPrediction = true;
        _pending.clear();
    }
}

void WorldState::reconcile(const Vec2 &serverPosition, std::uint16_t inputAck)
{
    while (!_pending.empty() &&
           !proto::isNewer(_pending.front().first, inputAck)) {
        _pending.pop_front();
    }
    _predicted = serverPosition;
    for (const auto &[id, keys] : _pending) {
        _predicted =
            rtype::game::moveShip(_predicted, keys, _rules,
                                  1.0F / static_cast<float>(_rules.tickRate));
    }
}

void WorldState::applySnapshot(const proto::Snapshot &snapshot, double now)
{
    for (const proto::EntityState &state : snapshot.entities) {
        Entity &entity = _entities[state.netId];
        const Vec2 position = pixels(state.x, state.y);

        entity.look.netId = state.netId;
        entity.look.position = position;
        entity.updatedAt = now;
        entity.samples.push_back({now, position});
        if (entity.samples.size() > MAX_SAMPLES) {
            entity.samples.pop_front();
        }
        if (_configured && isLocalShip(entity)) {
            _hasPrediction = true;
            reconcile(position, snapshot.inputAck);
        }
    }
}

void WorldState::apply(const proto::Message &message, double now)
{
    if (const auto *spawn = std::get_if<proto::Spawn>(&message)) {
        applySpawn(*spawn, now);
    } else if (const auto *destroy = std::get_if<proto::Destroy>(&message)) {
        _entities.erase(destroy->netId);
    } else if (const auto *snapshot = std::get_if<proto::Snapshot>(&message)) {
        applySnapshot(*snapshot, now);
    } else if (const auto *joined =
                   std::get_if<proto::PlayerJoined>(&message)) {
        _players[joined->playerIndex].present = true;
        _players[joined->playerIndex].name = joined->name;
    } else if (const auto *left = std::get_if<proto::PlayerLeft>(&message)) {
        _players[left->playerIndex] = PlayerInfo{};
    } else if (const auto *score = std::get_if<proto::Score>(&message)) {
        _players[score->playerIndex].score = score->score;
    } else if (const auto *state = std::get_if<proto::GameState>(&message)) {
        _phase = state->phase;
        _level = state->level;
    }
}

void WorldState::predict(std::uint16_t inputId, std::uint8_t keys)
{
    if (!_configured || !_hasPrediction || inputId == 0) {
        return;
    }
    _pending.emplace_back(inputId, keys);
    _predicted = rtype::game::moveShip(
        _predicted, keys, _rules, 1.0F / static_cast<float>(_rules.tickRate));
}

void WorldState::update(double now)
{
    std::erase_if(_entities, [this, now](const auto &item) {
        return !isLocalShip(item.second) &&
               now - item.second.updatedAt > STALE_AFTER;
    });
}

namespace {

template <typename Samples>
std::size_t firstAtOrAfter(const Samples &samples, double time)
{
    std::size_t index = 0;

    while (index < samples.size() && samples[index].time < time) {
        ++index;
    }
    return index;
}

Vec2 lerpAt(Vec2 from, Vec2 to, double span, double elapsed)
{
    const auto ratio = static_cast<float>(elapsed / std::max(span, 1e-6));

    return {from.x + (to.x - from.x) * ratio, from.y + (to.y - from.y) * ratio};
}

Vec2 extrapolate(Vec2 last, Vec2 velocity, double ahead)
{
    const auto seconds =
        static_cast<float>(std::min(ahead, WorldState::EXTRAPOLATION_LIMIT));

    return {last.x + velocity.x * seconds, last.y + velocity.y * seconds};
}

} // namespace

Vec2 WorldState::interpolate(const Entity &entity, double renderTime)
{
    const auto &samples = entity.samples;

    if (samples.empty()) {
        return entity.look.position;
    }
    const std::size_t next = firstAtOrAfter(samples, renderTime);

    if (next == 0) {
        return samples.front().position;
    }
    if (next < samples.size()) {
        return lerpAt(samples[next - 1].position, samples[next].position,
                      samples[next].time - samples[next - 1].time,
                      renderTime - samples[next - 1].time);
    }
    return extrapolate(samples.back().position, entity.velocity,
                       renderTime - samples.back().time);
}

std::vector<RenderEntity> WorldState::visible(double now) const
{
    std::vector<RenderEntity> out;

    out.reserve(_entities.size());
    for (const auto &[netId, entity] : _entities) {
        RenderEntity item = entity.look;

        item.position = _hasPrediction && isLocalShip(entity)
                            ? _predicted
                            : interpolate(entity, now - INTERPOLATION_DELAY);
        out.push_back(item);
    }
    return out;
}

bool WorldState::configured() const noexcept
{
    return _configured;
}

std::uint8_t WorldState::localSlot() const noexcept
{
    return _localSlot;
}

const rtype::game::GameRules &WorldState::rules() const noexcept
{
    return _rules;
}

const std::array<PlayerInfo, proto::MAX_PLAYERS> &
WorldState::players() const noexcept
{
    return _players;
}

std::uint8_t WorldState::phase() const noexcept
{
    return _phase;
}

std::uint8_t WorldState::level() const noexcept
{
    return _level;
}

std::size_t WorldState::entityCount() const noexcept
{
    return _entities.size();
}

} /* namespace client::world */
