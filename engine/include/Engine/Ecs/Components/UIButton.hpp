/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIButton.hpp
*/

#ifndef UIBUTTON_HPP_
#define UIBUTTON_HPP_

#include <cstddef>
#include <cstdint>
#include <string>

#include "Engine/Ecs/Components/UIColor.hpp"

namespace engine::ecs::components {

/**
 * @brief Current interaction state of a button.
 */
enum class UIButtonState : std::uint8_t {
    NORMAL,
    HOVERED,
    PRESSED,
    DISABLED
};

/**
 * @brief Interactive button component for UI entities.
 *
 * The button is filled with the color of its state. An entity that also has a
 * UIPanel is drawn with the panel instead.
 */
struct UIButton {
    UIButtonState state = UIButtonState::NORMAL;
    std::string action = "";
    bool isFocused = false;

    UIColor normalColor{200, 200, 200, 255};
    UIColor hoveredColor{255, 255, 255, 255};
    UIColor pressedColor{150, 150, 150, 255};
    UIColor disabledColor{100, 100, 100, 180};
    UIColor focusedOutlineColor{255, 215, 0, 255};

    friend bool operator==(const UIButton &, const UIButton &) = default;
};

/**
 * @brief Event published when a button is successfully clicked or activated.
 */
struct UIButtonClickedEvent {
    std::size_t entityIndex = 0;
    std::string action;

    friend bool operator==(const UIButtonClickedEvent &,
                           const UIButtonClickedEvent &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIButton;
using components::UIButtonClickedEvent;
using components::UIButtonState;

} // namespace engine::ecs

#endif /* !UIBUTTON_HPP_ */
