/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudFeed.cpp
*/

#include "Client/Hud/HudFeed.hpp"

#include <cstddef>
#include <variant>

#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Components/UIText.hpp"

namespace client::hud {

namespace {

using events::CueDirection;
using events::CueIcon;
using events::EntityKind;
namespace proto = rtype::protocol;

constexpr std::size_t MAX_TRACKED = 1024;

bool validKind(std::uint8_t kind) noexcept
{
    return kind >= static_cast<std::uint8_t>(EntityKind::PLAYER_SHIP) &&
           kind <= static_cast<std::uint8_t>(EntityKind::BYDOS_MISSILE);
}

bool isKind(std::uint8_t kind, EntityKind expected) noexcept
{
    return kind == static_cast<std::uint8_t>(expected);
}

} // namespace

HudFeed::HudFeed(engine::core::EventBus *bus)
    : _ownedBus(bus == nullptr ? std::make_unique<engine::core::EventBus>()
                               : nullptr),
      _bus(bus == nullptr ? *_ownedBus : *bus),
      _controller(_bus)
{
    engine::ecs::registerUIComponents(_registry);
    _cues = std::make_unique<VisualCueIndicator>(_registry, _bus, "");
}

HudFeed::~HudFeed()
{
    _cues.reset();
}

void HudFeed::welcome(const proto::Welcome &welcome)
{
    _worldWidth = welcome.worldWidth;
    _bus.publish(
        events::WelcomeReceived{welcome.playerIndex, welcome.maxPlayers});
}

void HudFeed::feed(const proto::Message &message)
{
    if (const auto *spawn = std::get_if<proto::Spawn>(&message))
        onSpawn(*spawn);
    else if (const auto *destroy = std::get_if<proto::Destroy>(&message))
        onDestroy(*destroy);
    else if (const auto *snap = std::get_if<proto::Snapshot>(&message))
        onSnapshot(*snap);
    else if (const auto *score = std::get_if<proto::Score>(&message))
        _bus.publish(events::ScoreChanged{score->playerIndex, score->score});
    else if (const auto *join = std::get_if<proto::PlayerJoined>(&message))
        _bus.publish(events::PlayerJoined{join->playerIndex, join->name});
    else if (const auto *left = std::get_if<proto::PlayerLeft>(&message))
        _bus.publish(events::PlayerLeft{left->playerIndex, left->reason});
    else if (const auto *state = std::get_if<proto::GameState>(&message))
        onGameState(*state);
}

void HudFeed::update(float dt, bool shootHeld)
{
    _controller.update(dt, shootHeld);
    _cues->update(dt);
}

const HudState &HudFeed::state() const noexcept
{
    return _controller.state();
}

std::string HudFeed::cue(CueDirection direction) const
{
    return _registry.getComponents<engine::ecs::components::UIText>()
        .get(_cues->label(direction))
        ->text;
}

void HudFeed::onSpawn(const proto::Spawn &spawn)
{
    if (!validKind(spawn.kind))
        return;
    const auto kind = static_cast<EntityKind>(spawn.kind);

    if (_tracked.size() < MAX_TRACKED)
        _tracked[spawn.netId] = {spawn.kind, spawn.x};
    _bus.publish(events::EntitySpawned{spawn.netId, kind, spawn.owner});
    if (kind == EntityKind::PLAYER_MISSILE &&
        state().localPlayer != spawn.owner)
        requestCue(CueIcon::SHOT, spawn.x);
    else if (kind == EntityKind::BYDOS_MISSILE)
        requestCue(CueIcon::ENEMY, spawn.x);
}

void HudFeed::onDestroy(const proto::Destroy &destroy)
{
    if (destroy.reason >
        static_cast<std::uint8_t>(events::DestroyReason::PICKED_UP))
        return;
    const auto reason = static_cast<events::DestroyReason>(destroy.reason);
    const auto found = _tracked.find(destroy.netId);

    _bus.publish(
        events::EntityDestroyed{destroy.netId, reason, destroy.killer});
    if (found == _tracked.end())
        return;
    const std::uint8_t kind = found->second.kind;

    if (reason == events::DestroyReason::KILLED &&
        (isKind(kind, EntityKind::PLAYER_SHIP) ||
         isKind(kind, EntityKind::BYDOS)))
        requestCue(CueIcon::EXPLOSION, found->second.x);
    _tracked.erase(found);
}

void HudFeed::onSnapshot(const proto::Snapshot &snapshot)
{
    for (const proto::EntityState &entity : snapshot.entities) {
        const auto found = _tracked.find(entity.netId);

        if (found != _tracked.end())
            found->second.x = entity.x;
    }
}

void HudFeed::onGameState(const proto::GameState &state)
{
    if (state.phase > static_cast<std::uint8_t>(events::GamePhase::GAME_OVER))
        return;
    _bus.publish(events::GameStateChanged{
        static_cast<events::GamePhase>(state.phase), state.level});
}

void HudFeed::requestCue(CueIcon icon, std::int16_t x)
{
    _bus.publish(events::VisualCueRequested{icon, sideOf(x)});
}

CueDirection HudFeed::sideOf(std::int16_t x) const noexcept
{
    const bool left = x < static_cast<int>(_worldWidth) / 2;

    return left ? CueDirection::LEFT : CueDirection::RIGHT;
}

} // namespace client::hud
