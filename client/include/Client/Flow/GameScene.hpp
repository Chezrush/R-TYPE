/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameScene.hpp
*/

#ifndef GAMESCENE_HPP_
#define GAMESCENE_HPP_

#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "Client/Flow/IGameDevice.hpp"
#include "Client/Hud/HudFeed.hpp"
#include "Client/Hud/HudView.hpp"
#include "Client/Net/ClientSession.hpp"
#include "Client/Render/Frame.hpp"
#include "Client/Scenes/GameOverlays.hpp"
#include "Client/Scenes/Starfield.hpp"
#include "Client/Settings/ClientSettings.hpp"
#include "Client/World/WorldState.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Core/IScene.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"
#include "Timing.hpp"

namespace client::flow {

/** @brief What the game scene needs from the application. */
struct GameContext {
    std::shared_ptr<client::net::ClientSession> session;
    IGameDevice &device;
    const settings::ClientSettings &settings;
    engine::ecs::Registry &registry;
    engine::core::EventBus &bus;
    engine::ecs::UIInteractionSystem &interaction;
    std::string fontId;
    float width = 1280.0f;
};

/**
 * @brief The game itself: network, prediction, interpolation and the HUD.
 *
 * Same behavior as the former game loop: it reads the session, applies the
 * messages to the world, sends the inputs at the tick rate of the server and
 * predicts the own ship. The overlays (menu, respawn, game over) come from
 * GameOverlays through the scene router; while the menu or the game over
 * screen is up, no key is sent. The session says goodbye when the scene
 * exits. If the session breaks, ConnectionLost is published once.
 */
class GameScene : public engine::core::IScene {
public:
    /**
     * @param active Set to this scene while it lives, so the application can
     * ask it for a frame.
     */
    GameScene(GameContext context, GameScene *&active);
    ~GameScene() override;

    GameScene(const GameScene &) = delete;
    GameScene &operator=(const GameScene &) = delete;
    GameScene(GameScene &&) = delete;
    GameScene &operator=(GameScene &&) = delete;

    void onEnter() override;
    void onExit() override;
    void onPause() override;
    void onResume() override;
    void update(float dt) override;
    void render() override;
    void handleInput() override;
    [[nodiscard]] bool isOverlay() const noexcept override;

    /** @brief What the window draws now. */
    [[nodiscard]] render::Frame frame() const;

    [[nodiscard]] const scenes::GameOverlays &overlays() const noexcept;
    [[nodiscard]] const hud::HudState &hudState() const noexcept;

private:
    void listen();
    void onKey(settings::ScanCode key);
    void receive();
    void sendInputs(rtype::protocol::TimePoint tick);
    void updateHud(float dt);
    void syncMarkers();
    void publishStats(float dt);
    void checkSession();
    [[nodiscard]] std::uint8_t currentKeys() const;
    [[nodiscard]] double stepSeconds() const;

    GameContext _context;
    GameScene *&_active;
    hud::HudFeed _feed;
    scenes::GameOverlays _overlays;
    hud::HudView _view;
    world::WorldState _world;
    scenes::Starfield _stars{120, 7};
    std::array<engine::core::EventBus::ListenerId, 2> _listeners{};
    rtype::protocol::TimePoint _start;
    double _now = 0.0;
    double _accumulator = 0.0;
    double _previous = 0.0;
    float _sinceStats = 0.0f;
    bool _lost = false;
};

} // namespace client::flow

#endif /* !GAMESCENE_HPP_ */
