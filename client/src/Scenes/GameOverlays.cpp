/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameOverlays.cpp
*/

#include "Client/Scenes/GameOverlays.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

#include "Client/Scenes/GameOverOverlay.hpp"
#include "Client/Scenes/InGameMenuOverlay.hpp"
#include "Client/Scenes/RespawnOverlay.hpp"

namespace client::scenes {

namespace {

using engine::ecs::components::UICancelEvent;

std::optional<SceneRoute> routeOf(GameOverlay overlay) noexcept
{
    switch (overlay) {
        case GameOverlay::IN_GAME_MENU:
            return SceneRoute::IN_GAME_MENU;
        case GameOverlay::RESPAWN:
            return SceneRoute::RESPAWN;
        case GameOverlay::GAME_OVER:
            return SceneRoute::GAME_OVER;
        default:
            return std::nullopt;
    }
}

} // namespace

template <typename Event>
void GameOverlays::listen(void (GameOverlays::*handler)(const Event &))
{
    _listeners.push_back(_bus.subscribe<Event>(
        [this, handler](const Event &event) { (this->*handler)(event); }));
}

GameOverlays::GameOverlays(engine::core::EventBus &bus)
    : _bus(bus)
{
    listen(&GameOverlays::onWelcome);
    listen(&GameOverlays::onSpawned);
    listen(&GameOverlays::onDestroyed);
    listen(&GameOverlays::onScore);
    listen(&GameOverlays::onGameState);
    listen(&GameOverlays::onCancel);
    listen(&GameOverlays::onResume);
    listen(&GameOverlays::onDisconnect);
}

GameOverlays::~GameOverlays()
{
    for (const auto listener : _listeners)
        _bus.unsubscribe(listener);
}

void GameOverlays::update(float dt) noexcept
{
    _respawnLeft = std::max(0.0f, _respawnLeft - dt);
}

GameOverlay GameOverlays::active() const noexcept
{
    return _active;
}

int GameOverlays::respawnSecondsLeft() const noexcept
{
    return static_cast<int>(std::ceil(_respawnLeft));
}

std::uint32_t GameOverlays::score() const noexcept
{
    return _score;
}

void GameOverlays::onWelcome(const events::WelcomeReceived &event)
{
    _player = event.playerIndex;
}

void GameOverlays::onSpawned(const events::EntitySpawned &event)
{
    if (event.kind != events::EntityKind::PLAYER_SHIP || event.owner != _player)
        return;
    _ship = event.netId;
    _dead = false;
    refresh();
}

void GameOverlays::onDestroyed(const events::EntityDestroyed &event)
{
    if (event.netId != _ship)
        return;
    _ship.reset();
    _dead = event.reason == events::DestroyReason::KILLED;
    _respawnLeft = _dead ? events::RESPAWN_DELAY_SECONDS : 0.0f;
    refresh();
}

void GameOverlays::onScore(const events::ScoreChanged &event)
{
    if (event.playerIndex == _player)
        _score = event.score;
}

void GameOverlays::onGameState(const events::GameStateChanged &event)
{
    _gameOver = event.phase == events::GamePhase::GAME_OVER;
    refresh();
}

void GameOverlays::onCancel(const UICancelEvent & /*event*/)
{
    if (_active == GameOverlay::NONE || _active == GameOverlay::RESPAWN) {
        _menuWanted = true;
        refresh();
    }
}

void GameOverlays::onResume(const events::ResumeRequested & /*event*/)
{
    _menuWanted = false;
    refresh();
}

void GameOverlays::onDisconnect(const events::DisconnectRequested & /*event*/)
{
    // The navigation that follows replaces the whole stack.
    if (_active == GameOverlay::IN_GAME_MENU)
        _bus.publish(events::InGameMenuClosed{});
    _left = true;
    _active = GameOverlay::NONE;
}

void GameOverlays::refresh()
{
    if (_left)
        return;
    if (_gameOver) {
        _menuWanted = false;
        show(GameOverlay::GAME_OVER);
    } else if (_menuWanted) {
        show(GameOverlay::IN_GAME_MENU);
    } else {
        show(_dead ? GameOverlay::RESPAWN : GameOverlay::NONE);
    }
}

void GameOverlays::show(GameOverlay overlay)
{
    if (overlay == _active)
        return;
    if (_active != GameOverlay::NONE)
        _bus.publish(engine::core::OverlaysClearRequested{});
    if (_active == GameOverlay::IN_GAME_MENU)
        _bus.publish(events::InGameMenuClosed{});
    if (overlay == GameOverlay::IN_GAME_MENU)
        _bus.publish(events::InGameMenuOpened{});
    if (const auto route = routeOf(overlay))
        _bus.publish(OverlayRequested{*route});
    _active = overlay;
}

void addOverlayRoutes(SceneRouter &router,
                      const engine::ecs::UISceneContext &context,
                      const GameOverlays &overlays)
{
    router.setRoute(SceneRoute::IN_GAME_MENU, [context] {
        return std::make_unique<InGameMenuOverlay>(context);
    });
    router.setRoute(SceneRoute::RESPAWN, [context, &overlays] {
        return std::make_unique<RespawnOverlay>(context, overlays);
    });
    router.setRoute(SceneRoute::GAME_OVER, [context, &overlays] {
        return std::make_unique<GameOverOverlay>(context, overlays.score());
    });
}

} // namespace client::scenes
