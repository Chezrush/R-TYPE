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

namespace engine::ecs::components {

/**
 * @brief Current interaction state of a button.
 */
enum class UIButtonState {
    NORMAL,
    HOVERED,
    PRESSED,
    DISABLED
};

/**
 * @brief RGBA color for button styling.
 */
struct UIColor {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    [[nodiscard]] bool operator==(const UIColor &) const noexcept = default;
};

/**
 * @brief Interactive button component for UI entities.
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

    [[nodiscard]] bool operator==(const UIButton &) const noexcept = default;
};

/**
 * @brief Event published when a button is successfully clicked or activated.
 */
struct UIButtonClickedEvent {
    std::size_t entityIndex = 0;
    std::string action;

    [[nodiscard]] bool operator==(
        const UIButtonClickedEvent &
    ) const noexcept = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIButton;
using components::UIButtonClickedEvent;
using components::UIButtonState;
using components::UIColor;

} // namespace engine::ecs

#endif /* !UIBUTTON_HPP_ */

