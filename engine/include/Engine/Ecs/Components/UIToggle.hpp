/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIToggle.hpp
*/

#ifndef UITOGGLE_HPP_
#define UITOGGLE_HPP_

#include <string>

#include "Engine/Ecs/Components/UIWidgetStyle.hpp"

namespace engine::ecs::components {

/**
 * @brief Checkbox / on-off switch.
 *
 * Flipped by a click released over it, or by Enter / Space when focused. The
 * box is a square of the bounds height on the left; put a UIText on another
 * entity for its label.
 */
struct UIToggle {
    bool checked = false;
    bool isFocused = false;
    bool enabled = true;
    std::string action;
    UIWidgetStyle style;

    friend bool operator==(const UIToggle &, const UIToggle &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIToggle;

} // namespace engine::ecs

#endif /* !UITOGGLE_HPP_ */
