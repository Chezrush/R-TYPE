/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameScene.cpp
*/

#include "Client/Flow/GameScene.hpp"

#include <algorithm>
#include <chrono>
#include <optional>
#include <utility>

#include "Client/Events/ConnectionEvents.hpp"
#include "Client/Events/SettingsEvents.hpp"
#include "Client/Events/StatsEvents.hpp"
#include "Client/Flow/GameInput.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"

namespace client::flow {

namespace {

using client::net::ClientSession;
using rtype::protocol::Clock;

constexpr float SHIP_HALF_WIDTH = 49.5f;
constexpr float STATS_PERIOD = 0.25f;
constexpr double MAX_FRAME = 0.25;
constexpr std::uint8_t KIND_SHIP = 1;

double seconds(Clock::time_point start, Clock::time_point now)
{
    return std::chrono::duration<double>(now - start).count();
}

} // namespace

GameScene::GameScene(GameContext context, GameScene *&active)
    : _context(std::move(context)),
      _active(active),
      _feed(&_context.bus),
      _overlays(_context.bus),
      _view(_context.registry, _context.fontId, "sprites/r-typesheet42.gif",
            _context.width),
      _start(Clock::now())
{
    listen();
    _context.bus.publish(
        events::ReducedMotionChanged{_context.settings.reducedMotion});
}

void GameScene::listen()
{
    _listeners[0] = _context.bus.subscribe<events::WindowResized>(
        [this](const events::WindowResized &event) {
            _view.resize(event.width);
        });
    _listeners[1] = _context.bus.subscribe<events::RawKeyPressed>(
        [this](const events::RawKeyPressed &event) { onKey(event.key); });
}

void GameScene::onKey(settings::ScanCode key)
{
    const auto menuKey = settings::bindingOf(_context.settings.bindings,
                                             settings::GameAction::OPEN_MENU);
    const auto shown = _overlays.active();
    const bool free = shown == scenes::GameOverlay::NONE ||
                      shown == scenes::GameOverlay::RESPAWN;

    if (free && key == menuKey.key && key != settings::KEY_ESCAPE) {
        _context.bus.publish(engine::ecs::components::UICancelEvent{});
    }
}

GameScene::~GameScene()
{
    for (const auto listener : _listeners) {
        _context.bus.unsubscribe(listener);
    }
    if (_active == this) {
        _active = nullptr;
    }
}

void GameScene::onEnter()
{
    _active = this;
}

void GameScene::onExit()
{
    _context.session->disconnect(Clock::now());
    if (_active == this) {
        _active = nullptr;
    }
}

void GameScene::onPause() {}

void GameScene::onResume() {}

void GameScene::render() {}

bool GameScene::isOverlay() const noexcept
{
    return false;
}

void GameScene::handleInput()
{
    _context.interaction.process(_context.registry);
}

double GameScene::stepSeconds() const
{
    return 1.0 / static_cast<double>(_world.rules().tickRate);
}

std::uint8_t GameScene::currentKeys() const
{
    const auto shown = _overlays.active();

    if (shown == scenes::GameOverlay::IN_GAME_MENU ||
        shown == scenes::GameOverlay::GAME_OVER) {
        return 0;
    }
    return readKeys(_context.device, _context.settings);
}

void GameScene::update(float dt)
{
    const auto tick = Clock::now();

    _now = seconds(_start, tick);
    _overlays.update(dt);
    _context.session->update(tick);
    receive();
    sendInputs(tick);
    updateHud(dt);
    if (!_context.settings.reducedMotion) {
        _stars.advance(static_cast<float>(stepSeconds()));
    }
    _world.update(_now);
    syncMarkers();
    publishStats(dt);
    checkSession();
}

void GameScene::receive()
{
    ClientSession &session = *_context.session;

    if (!_world.configured() && session.welcome()) {
        _world.configure(*session.welcome());
        _feed.welcome(*session.welcome());
    }
    for (const auto &message : session.takeMessages()) {
        _world.apply(message, _now);
        _feed.feed(message);
        playSounds(message, _context.device);
    }
}

void GameScene::sendInputs(rtype::protocol::TimePoint tick)
{
    _accumulator += std::min(_now - _previous, MAX_FRAME);
    _previous = _now;
    while (_accumulator >= stepSeconds() && _world.configured()) {
        const std::uint8_t keys = currentKeys();

        _world.predict(_context.session->sendInput(keys, tick), keys);
        _accumulator -= stepSeconds();
    }
}

void GameScene::updateHud(float dt)
{
    const bool shooting = (currentKeys() & rtype::protocol::KEY_SHOOT) != 0;

    _feed.update(std::min(dt, static_cast<float>(MAX_FRAME)), shooting);
    _view.sync(_feed.state());
}

void GameScene::syncMarkers()
{
    std::array<std::optional<engine::core::Vec2>, hud::MAX_PLAYERS> tops{};

    for (const auto &entity : _world.visible(_now)) {
        if (entity.kind == KIND_SHIP && entity.owner < tops.size()) {
            tops[entity.owner] = _context.device.toScreen(
                {entity.position.x + SHIP_HALF_WIDTH, entity.position.y});
        }
    }
    for (std::size_t player = 0; player < tops.size(); ++player) {
        _view.setMarker(player, tops[player]);
    }
}

void GameScene::publishStats(float dt)
{
    _sinceStats += dt;
    if (_sinceStats < STATS_PERIOD) {
        return;
    }
    _sinceStats = 0.0f;
    events::NetStatsUpdated stats;

    stats.rttMs = static_cast<float>(_context.session->rtt().count());
    _context.bus.publish(stats);
    _context.bus.publish(
        events::StatsLine{"Entities", std::to_string(_world.entityCount())});
}

void GameScene::checkSession()
{
    const auto state = _context.session->state();

    if (_lost || state == ClientSession::State::CONNECTED ||
        state == ClientSession::State::CLOSED) {
        return;
    }
    _lost = true;
    _context.bus.publish(events::ConnectionLost{});
}

render::Frame GameScene::frame() const
{
    render::Frame result{_world,
                         _stars,
                         _now,
                         bannerOf(_context.session->state()),
                         static_cast<int>(_context.session->rtt().count()),
                         _context.settings.palette,
                         _context.settings.highContrast};

    for (std::size_t side = 0; side < result.cues.size(); ++side) {
        result.cues[side] = _feed.cue(static_cast<events::CueDirection>(side));
    }
    return result;
}

const scenes::GameOverlays &GameScene::overlays() const noexcept
{
    return _overlays;
}

const hud::HudState &GameScene::hudState() const noexcept
{
    return _feed.state();
}

} // namespace client::flow
