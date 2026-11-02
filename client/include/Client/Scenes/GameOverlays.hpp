/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameOverlays.hpp
*/

#ifndef GAMEOVERLAYS_HPP_
#define GAMEOVERLAYS_HPP_

#include <cstdint>
#include <optional>
#include <vector>

#include "Client/Events/GameEvents.hpp"
#include "Client/Events/OverlayEvents.hpp"
#include "Client/Scenes/SceneRoute.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"
#include "Engine/Ecs/UIScene.hpp"

namespace client::scenes {

/** @brief Overlay shown above the game; there is never more than one. */
enum class GameOverlay : std::uint8_t {
    NONE,
    IN_GAME_MENU,
    RESPAWN,
    GAME_OVER
};

/**
 * @brief Decides which overlay is above the game, from events only.
 *
 * Game over wins over the in-game menu, which wins over the respawn countdown.
 * The countdown keeps running under the menu and shows again when it closes, if
 * our ship is still not back. The game scene owns this object, ticks update()
 * and must run the input (Escape). The changes go through the SceneRouter, so
 * they happen between two frames.
 */
class GameOverlays {
public:
    explicit GameOverlays(engine::core::EventBus &bus);
    ~GameOverlays();

    GameOverlays(const GameOverlays &) = delete;
    GameOverlays &operator=(const GameOverlays &) = delete;
    GameOverlays(GameOverlays &&) = delete;
    GameOverlays &operator=(GameOverlays &&) = delete;

    /** @brief Advances the respawn countdown. */
    void update(float dt) noexcept;

    [[nodiscard]] GameOverlay active() const noexcept;

    /** @brief Whole seconds left before the respawn, rounded up. */
    [[nodiscard]] int respawnSecondsLeft() const noexcept;

    [[nodiscard]] std::uint32_t score() const noexcept;

private:
    template <typename Event>
    void listen(void (GameOverlays::*handler)(const Event &));

    void onWelcome(const events::WelcomeReceived &event);
    void onSpawned(const events::EntitySpawned &event);
    void onDestroyed(const events::EntityDestroyed &event);
    void onScore(const events::ScoreChanged &event);
    void onGameState(const events::GameStateChanged &event);
    void onCancel(const engine::ecs::components::UICancelEvent &event);
    void onResume(const events::ResumeRequested &event);
    void onDisconnect(const events::DisconnectRequested &event);
    void refresh();
    void show(GameOverlay overlay);

    engine::core::EventBus &_bus;
    std::vector<engine::core::EventBus::ListenerId> _listeners;
    std::optional<std::uint8_t> _player;
    std::optional<std::uint16_t> _ship;
    std::uint32_t _score = 0;
    float _respawnLeft = 0.0f;
    bool _dead = false;
    bool _gameOver = false;
    bool _menuWanted = false;
    bool _left = false;
    GameOverlay _active = GameOverlay::NONE;
};

/** @brief Gives the router the scenes of the three overlays. */
void addOverlayRoutes(SceneRouter &router,
                      const engine::ecs::UISceneContext &context,
                      const GameOverlays &overlays);

} // namespace client::scenes

#endif /* !GAMEOVERLAYS_HPP_ */
