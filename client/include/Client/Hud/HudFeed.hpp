/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudFeed.hpp
*/

#ifndef HUDFEED_HPP_
#define HUDFEED_HPP_

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "Client/Hud/HudController.hpp"
#include "Client/Hud/VisualCueIndicator.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Messages.hpp"

namespace client::hud {

/**
 * @brief Turns the server messages into the events of the HUD.
 *
 * It owns the bus, the HudController and the VisualCueIndicator, so the
 * game loop only feeds it the messages it already receives. Shots of the
 * other ships and kills become visual cues on the side they happen.
 */
class HudFeed {
public:
    HudFeed();
    ~HudFeed();

    HudFeed(const HudFeed &) = delete;
    HudFeed &operator=(const HudFeed &) = delete;
    HudFeed(HudFeed &&) = delete;
    HudFeed &operator=(HudFeed &&) = delete;

    /** @brief Announces our player slot and the width of the world. */
    void welcome(const rtype::protocol::Welcome &welcome);

    /** @brief Translates one server message into HUD events. */
    void feed(const rtype::protocol::Message &message);

    /** @brief Advances the cue timers and the beam charge. */
    void update(float dt, bool shootHeld);

    [[nodiscard]] const HudState &state() const noexcept;

    /** @brief The text shown on a side of the screen, empty if none. */
    [[nodiscard]] std::string cue(events::CueDirection direction) const;

private:
    struct Tracked {
        std::uint8_t kind = 0;
        std::int16_t x = 0;
    };

    void onSpawn(const rtype::protocol::Spawn &spawn);
    void onDestroy(const rtype::protocol::Destroy &destroy);
    void onSnapshot(const rtype::protocol::Snapshot &snapshot);
    void onGameState(const rtype::protocol::GameState &state);
    void requestCue(events::CueIcon icon, std::int16_t x);
    [[nodiscard]] events::CueDirection sideOf(std::int16_t x) const noexcept;

    engine::core::EventBus _bus;
    engine::ecs::Registry _registry;
    HudController _controller;
    std::unique_ptr<VisualCueIndicator> _cues;
    std::uint16_t _worldWidth = 1280;
    std::map<std::uint16_t, Tracked> _tracked;
};

} // namespace client::hud

#endif /* !HUDFEED_HPP_ */
