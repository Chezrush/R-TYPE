/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIColor.hpp
*/

#ifndef UICOLOR_HPP_
#define UICOLOR_HPP_

#include <cstdint>

namespace engine::ecs::components {

/**
 * @brief RGBA color shared by every UI component.
 */
struct UIColor {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    friend bool operator==(const UIColor &, const UIColor &) = default;
};

/**
 * @brief Named use of a color, resolved by the UITheme when drawing.
 *
 * NONE keeps the color stored in the component.
 */
enum class UIColorRole : std::uint8_t {
    NONE,
    TEXT,
    TEXT_MUTED,
    TEXT_ON_PLAYER,
    BACKGROUND,
    SURFACE,
    BUTTON,
    BUTTON_HOVERED,
    BUTTON_PRESSED,
    BUTTON_DISABLED,
    ACCENT,
    FOCUS,
    WARNING,
    ERROR_TEXT,
    GAUGE,
    GAUGE_FULL,
    PLAYER_1,
    PLAYER_2,
    PLAYER_3,
    PLAYER_4,
    COUNT
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIColorRole;

using components::UIColor;

} // namespace engine::ecs

#endif /* !UICOLOR_HPP_ */
