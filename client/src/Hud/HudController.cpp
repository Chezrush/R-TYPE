/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudController.cpp
*/

#include "Client/Hud/HudController.hpp"

#include <algorithm>

namespace client::hud {

template <typename Event>
void HudController::listen(void (HudController::*handler)(const Event &))
{
    _listeners.push_back(_bus.subscribe<Event>(
        [this, handler](const Event &event) { (this->*handler)(event); }));
}

HudController::HudController(engine::core::EventBus &bus,
                             float beamChargeSeconds)
    : _bus(bus),
      _beam(beamChargeSeconds)
{
    listen(&HudController::onWelcome);
    listen(&HudController::onEntitySpawned);
    listen(&HudController::onEntityDestroyed);
    listen(&HudController::onPlayerJoined);
    listen(&HudController::onPlayerLeft);
    listen(&HudController::onScoreChanged);
    listen(&HudController::onGameStateChanged);
    listen(&HudController::onReducedMotion);
}

HudController::~HudController()
{
    for (const auto id : _listeners)
        _bus.unsubscribe(id);
}

void HudController::update(float dt, bool shootHeld) noexcept
{
    _beam.update(dt, shootHeld && _state.localAlive);
    _state.beamCharge = _beam.charge();
}

const HudState &HudController::state() const noexcept
{
    return _state;
}

void HudController::onWelcome(const events::WelcomeReceived &event)
{
    if (event.playerIndex >= MAX_PLAYERS)
        return;
    _state.localPlayer = event.playerIndex;
    setConnected(event.playerIndex, true);
}

void HudController::onEntitySpawned(const events::EntitySpawned &event)
{
    if (event.kind != events::EntityKind::PLAYER_SHIP)
        return;
    setConnected(event.owner, true);
    if (!_state.localPlayer.has_value() || event.owner != _state.localPlayer)
        return;
    _state.localShip = event.netId;
    _state.localAlive = true;
}

void HudController::onEntityDestroyed(const events::EntityDestroyed &event)
{
    if (!_state.localAlive || _state.localShip != event.netId)
        return;
    if (event.reason == events::DestroyReason::KILLED)
        _state.lives = std::max(0, _state.lives - 1);
    _state.localShip.reset();
    _state.localAlive = false;
    _beam.reset();
    _state.beamCharge = 0.0f;
}

void HudController::onPlayerJoined(const events::PlayerJoined &event)
{
    setConnected(event.playerIndex, true);
}

void HudController::onPlayerLeft(const events::PlayerLeft &event)
{
    setConnected(event.playerIndex, false);
}

void HudController::onScoreChanged(const events::ScoreChanged &event)
{
    if (_state.localPlayer != event.playerIndex)
        return;
    _state.score = event.score;
}

void HudController::onGameStateChanged(const events::GameStateChanged &event)
{
    _state.phase = event.phase;
    _state.level = event.level;
}

void HudController::onReducedMotion(const events::ReducedMotionChanged &event)
{
    _state.reducedMotion = event.reduced;
}

void HudController::setConnected(std::uint8_t player, bool connected) noexcept
{
    if (player < MAX_PLAYERS)
        _state.connected.at(player) = connected;
}

} // namespace client::hud
