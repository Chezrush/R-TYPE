/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientApp.cpp
*/

#include "Client/Flow/ClientApp.hpp"

#include <utility>

#include "Client/Events/GameEvents.hpp"
#include "Client/Events/SettingsEvents.hpp"
#include "Client/Flow/GameInput.hpp"
#include "Client/Scenes/ConnectScene.hpp"
#include "Client/Scenes/ControlsScene.hpp"
#include "Client/Scenes/GameOverOverlay.hpp"
#include "Client/Scenes/HowToPlayScene.hpp"
#include "Client/Scenes/InGameMenuOverlay.hpp"
#include "Client/Scenes/MainMenuScene.hpp"
#include "Client/Scenes/MenuContext.hpp"
#include "Client/Scenes/NoticeOverlay.hpp"
#include "Client/Scenes/RespawnOverlay.hpp"
#include "Client/Scenes/SettingsScene.hpp"

namespace client::flow {

namespace {

using client::net::ClientSession;
using engine::core::IScene;
using scenes::SceneRoute;

constexpr float PERCENT = 100.0f;

engine::ecs::UISceneContext makeContext(AppServices &services)
{
    auto context = scenes::makeMenuContext(services.registry, services.bus,
                                           services.interaction,
                                           services.fontId, &services.theme);

    context.viewport = {services.layout.windowWidth(),
                        services.layout.windowHeight()};
    services.layout.setScale(
        static_cast<float>(services.store.settings().uiScalePercent) / PERCENT);
    return context;
}

} // namespace

ClientApp::ClientApp(AppServices services)
    : _services(std::move(services)),
      _context(makeContext(_services)),
      _router(_scenes, _services.bus),
      _stats(_services.registry, _services.bus, _services.fontId)
{
    registerRoutes();
    listen();
    _services.bus.publish(scenes::NavigationRequested{SceneRoute::MAIN_MENU});
    _router.apply();
    _services.layout.update(_services.registry);
}

ClientApp::~ClientApp()
{
    _scenes.clear();
    for (const auto listener : _listeners) {
        _services.bus.unsubscribe(listener);
    }
}

void ClientApp::registerRoutes()
{
    auto &store = _services.store;

    _router.setRoute(SceneRoute::MAIN_MENU, [this] { return makeMainMenu(); });
    _router.setRoute(SceneRoute::CONNECT, [this] {
        return std::make_unique<scenes::ConnectScene>(_context,
                                                      _services.prefill);
    });
    _router.setRoute(SceneRoute::GAME, [this] { return makeGame(); });
    _router.setRoute(SceneRoute::IN_GAME_MENU, [this] {
        return std::make_unique<scenes::InGameMenuOverlay>(_context);
    });
    _router.setRoute(SceneRoute::RESPAWN, [this] { return makeRespawn(); });
    _router.setRoute(SceneRoute::GAME_OVER, [this] { return makeGameOver(); });
    _router.setRoute(SceneRoute::NOTICE, [this] { return makeNotice(); });
    scenes::addSettingsRoute(_router, _context, store, _services.layout,
                             _services.theme);
    scenes::addControlsRoute(_router, _context, store);
    scenes::addHowToPlayRoute(_router, _context, store.settings().bindings);
}

void ClientApp::listen()
{
    auto &bus = _services.bus;

    _listeners[0] = bus.subscribe<events::ConnectRequested>(
        [this](const events::ConnectRequested &request) {
            onConnect(request);
        });
    _listeners[1] = bus.subscribe<events::ConnectCancelled>(
        [this](const events::ConnectCancelled &) { onCancel(); });
    _listeners[2] = bus.subscribe<events::ConnectionLost>(
        [this](const events::ConnectionLost &) { onLost(); });
    _listeners[3] = bus.subscribe<events::WindowResized>(
        [this](const events::WindowResized &size) {
            _context.viewport = {size.width, size.height};
        });
}

void ClientApp::frame(float dt)
{
    _services.layout.update(_services.registry);
    _scenes.handleInput();
    watchHandshake();
    _scenes.update(dt);
    _stats.update(dt);
    _router.apply();
    _services.layout.update(_services.registry);
}

void ClientApp::onConnect(const events::ConnectRequested &request)
{
    _services.prefill = {request.host, std::to_string(request.port),
                         request.nickname};
    _session = std::make_shared<ClientSession>(request.host, request.port,
                                               request.nickname);
    _connecting = true;
    _services.bus.publish(
        events::ConnectionStarted{request.host, request.port});
    if (_session->state() == ClientSession::State::FAILED) {
        failConnection(events::ConnectionFailure::INVALID_ADDRESS);
    }
}

void ClientApp::onCancel()
{
    _session.reset();
    _connecting = false;
}

void ClientApp::onLost()
{
    _notice = "The connection to the server was lost.";
    _services.bus.publish(scenes::NavigationRequested{SceneRoute::MAIN_MENU});
    _services.bus.publish(scenes::OverlayRequested{SceneRoute::NOTICE});
}

void ClientApp::failConnection(events::ConnectionFailure reason)
{
    _session.reset();
    _connecting = false;
    _services.bus.publish(events::ConnectionFailed{reason});
}

void ClientApp::watchHandshake()
{
    if (!_connecting || !_session) {
        return;
    }
    _session->update(rtype::protocol::Clock::now());
    const auto state = _session->state();

    if (state == ClientSession::State::CONNECTED && _session->welcome()) {
        _connecting = false;
        _services.bus.publish(events::WelcomeReceived{
            _session->welcome()->playerIndex, _session->welcome()->maxPlayers});
    } else if (state != ClientSession::State::CONNECTING &&
               state != ClientSession::State::CONNECTED) {
        failConnection(failureOf(state, _session->rejectReason()));
    }
}

std::unique_ptr<IScene> ClientApp::makeMainMenu()
{
    _session.reset();
    _connecting = false;
    return std::make_unique<scenes::MainMenuScene>(_context);
}

std::unique_ptr<IScene> ClientApp::makeGame()
{
    if (!_session) {
        return makeMainMenu();
    }
    GameContext context{_session,
                        _services.device,
                        _services.store.settings(),
                        _services.registry,
                        _services.bus,
                        _services.interaction,
                        _services.fontId,
                        _services.layout.windowWidth()};

    return std::make_unique<GameScene>(std::move(context), _game);
}

std::unique_ptr<IScene> ClientApp::makeRespawn()
{
    if (_game == nullptr) {
        return makeNotice();
    }
    return std::make_unique<scenes::RespawnOverlay>(_context,
                                                    _game->overlays());
}

std::unique_ptr<IScene> ClientApp::makeGameOver()
{
    if (_game == nullptr) {
        return makeNotice();
    }
    return std::make_unique<scenes::GameOverOverlay>(_context,
                                                     _game->overlays().score());
}

std::unique_ptr<IScene> ClientApp::makeNotice()
{
    return std::make_unique<scenes::NoticeOverlay>(_context, _notice);
}

bool ClientApp::quitRequested() const noexcept
{
    return _router.quitRequested();
}

void ClientApp::shutdown()
{
    _scenes.clear();
}

std::optional<render::Frame> ClientApp::gameFrame() const
{
    if (_game == nullptr) {
        return std::nullopt;
    }
    return _game->frame();
}

const IScene &ClientApp::currentScene() const
{
    return _scenes.currentScene();
}

const std::string &ClientApp::notice() const noexcept
{
    return _notice;
}

bool ClientApp::isConnecting() const noexcept
{
    return _connecting;
}

const hud::StatsOverlay &ClientApp::stats() const noexcept
{
    return _stats;
}

} // namespace client::flow
