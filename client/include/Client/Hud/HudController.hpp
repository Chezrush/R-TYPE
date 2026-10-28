/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudController.hpp
*/

#ifndef HUDCONTROLLER_HPP_
#define HUDCONTROLLER_HPP_

#include <vector>

#include "Client/Events/GameEvents.hpp"
#include "Client/Hud/BeamGauge.hpp"
#include "Client/Hud/HudState.hpp"
#include "Engine/Core/EventBus.hpp"

namespace client::hud {

/**
 * @brief Keeps the HudState up to date from game events and local input.
 *
 * It only listens to the EventBus: it never talks to the network. Our ship
 * is the PLAYER_SHIP spawned with our player index as owner, on join and on
 * every respawn. Lives are still counted on the client (the protocol has no
 * lives field): our ship destroyed with reason KILLED costs one. The
 * listeners are removed on destruction, so the bus must outlive the
 * controller.
 */
class HudController {
public:
    explicit HudController(engine::core::EventBus &bus,
                           float beamChargeSeconds = 1.5f);
    ~HudController();

    HudController(const HudController &) = delete;
    HudController &operator=(const HudController &) = delete;
    HudController(HudController &&) = delete;
    HudController &operator=(HudController &&) = delete;

    /** @brief Advances the beam charge from the local shoot key. */
    void update(float dt, bool shootHeld) noexcept;

    [[nodiscard]] const HudState &state() const noexcept;

private:
    template <typename Event>
    void listen(void (HudController::*handler)(const Event &));

    void onWelcome(const events::WelcomeReceived &event);
    void onEntitySpawned(const events::EntitySpawned &event);
    void onEntityDestroyed(const events::EntityDestroyed &event);
    void onPlayerJoined(const events::PlayerJoined &event);
    void onPlayerLeft(const events::PlayerLeft &event);
    void onScoreChanged(const events::ScoreChanged &event);
    void onGameStateChanged(const events::GameStateChanged &event);
    void setConnected(std::uint8_t player, bool connected) noexcept;

    engine::core::EventBus &_bus;
    std::vector<engine::core::EventBus::ListenerId> _listeners;
    HudState _state;
    BeamGauge _beam;
};

} // namespace client::hud

#endif /* !HUDCONTROLLER_HPP_ */
