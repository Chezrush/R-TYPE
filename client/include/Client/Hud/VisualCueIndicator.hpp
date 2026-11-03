/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** VisualCueIndicator.hpp
*/

#ifndef VISUALCUEINDICATOR_HPP_
#define VISUALCUEINDICATOR_HPP_

#include <array>
#include <string>

#include "Client/Events/GameEvents.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/Registry.hpp"

namespace client::hud {

/**
 * @brief Shows sounds on screen: a label on the side they come from.
 *
 * Each side shows the latest VisualCueRequested for a short time. It never
 * blinks or moves, so it also suits the reduced motion option. The entities and
 * the listener are removed on destruction: the registry and the bus must
 * outlive this object. Tick update() with the game.
 */
class VisualCueIndicator {
public:
    VisualCueIndicator(engine::ecs::Registry &registry,
                       engine::core::EventBus &bus, const std::string &fontId,
                       float durationSeconds = 1.5f);
    ~VisualCueIndicator();

    VisualCueIndicator(const VisualCueIndicator &) = delete;
    VisualCueIndicator &operator=(const VisualCueIndicator &) = delete;
    VisualCueIndicator(VisualCueIndicator &&) = delete;
    VisualCueIndicator &operator=(VisualCueIndicator &&) = delete;

    /** @brief Hides the cues whose time is over. */
    void update(float dt);

    /** @brief The label of a side of the screen. */
    [[nodiscard]] engine::ecs::Entity
    label(events::CueDirection direction) const;

    /** @brief "<< Explosion", "Enemy >>", "^ Shot", "v Alert"... */
    [[nodiscard]] static std::string cueText(events::CueIcon icon,
                                             events::CueDirection direction);

private:
    struct Side {
        engine::ecs::Entity entity;
        float timeLeft = 0.0f;
    };

    void build(events::CueDirection direction, const std::string &fontId);
    void show(const events::VisualCueRequested &cue);
    void setVisible(const Side &side, const std::string &text);

    engine::ecs::Registry &_registry;
    engine::core::EventBus &_bus;
    engine::core::EventBus::ListenerId _listener;
    float _duration;
    std::array<Side, 4> _sides;
};

} // namespace client::hud

#endif /* !VISUALCUEINDICATOR_HPP_ */
