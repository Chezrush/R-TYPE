/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIProgressBar.hpp
*/

#ifndef UIPROGRESSBAR_HPP_
#define UIPROGRESSBAR_HPP_

#include "Engine/Ecs/Components/UIColor.hpp"

namespace engine::ecs::components {

/**
 * @brief Growth direction of a progress bar fill.
 *
 * HORIZONTAL fills from left to right, VERTICAL from bottom to top.
 */
enum class UIFillDirection {
    HORIZONTAL,
    VERTICAL
};

/**
 * @brief Gauge (health, beam charge) filling the entity's UITransform bounds.
 *
 * value is clamped to [min, max] when drawn, and an empty range draws an
 * empty bar.
 */
struct UIProgressBar {
    float min = 0.0f;
    float max = 1.0f;
    float value = 0.0f;
    UIFillDirection direction = UIFillDirection::HORIZONTAL;
    UIColor backgroundColor{40, 40, 40, 200};
    UIColor fillColor{80, 200, 120, 255};

    [[nodiscard]] bool
    operator==(const UIProgressBar &) const noexcept = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIFillDirection;
using components::UIProgressBar;

} // namespace engine::ecs

#endif /* !UIPROGRESSBAR_HPP_ */
