/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UISlider.hpp
*/

#ifndef UISLIDER_HPP_
#define UISLIDER_HPP_

#include <string>

#include "Engine/Ecs/Components/UIWidgetStyle.hpp"

namespace engine::ecs::components {

/**
 * @brief Horizontal slider choosing a value in [min, max].
 *
 * The value is kept on a multiple of step from min (step <= 0 means
 * continuous). Left / Right move it by one step, a click or a drag puts it
 * under the pointer.
 */
struct UISlider {
    float min = 0.0f;
    float max = 1.0f;
    float step = 0.1f;
    float value = 0.0f;
    bool isFocused = false;
    bool enabled = true;
    std::string action;
    UIWidgetStyle style;

    friend bool operator==(const UISlider &, const UISlider &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UISlider;

} // namespace engine::ecs

#endif /* !UISLIDER_HPP_ */
