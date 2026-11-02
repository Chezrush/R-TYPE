/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UITheme.hpp
*/

#ifndef UITHEME_HPP_
#define UITHEME_HPP_

#include <array>
#include <cstddef>
#include <optional>

#include "Engine/Ecs/Components/UIColor.hpp"
#include "Engine/Ecs/Components/UIWidgetStyle.hpp"

namespace engine::ecs::components {

/** @brief Accessibility options of a theme. */
struct UIThemeOptions {
    /** @brief Draws a solid plate behind text. */
    bool textPlates = false;
    /** @brief Width of the frame around the focused widget. */
    float focusThickness = 2.0f;
    /** @brief Width of the frame around gauges; 0 draws none. */
    float gaugeOutline = 0.0f;
    /** @brief Asks for no flashing or animation. Read by the UI code. */
    bool reducedMotion = false;
};

/**
 * @brief Colors and accessibility options applied when the UI is drawn.
 *
 * Swapping the theme recolors the UI without touching the widgets. The engine
 * defines no color: a role without a color keeps the one of the component, so
 * an empty theme changes nothing.
 */
class UITheme {
public:
    void setColor(UIColorRole role, UIColor color) noexcept;

    /** @brief The color of a role, or the fallback if it has none. */
    [[nodiscard]] UIColor resolve(UIColorRole role,
                                  UIColor fallback) const noexcept;

    /** @brief A copy of the style with the colors of the theme. */
    [[nodiscard]] UIWidgetStyle styled(const UIWidgetStyle &style) const;

    /**
     * @brief Whether a text gets a plate.
     *
     * Not TEXT_ON_PLAYER: that text already sits on its own solid color.
     */
    [[nodiscard]] bool hasPlate(UIColorRole textRole) const noexcept;

    [[nodiscard]] UIThemeOptions &options() noexcept;
    [[nodiscard]] const UIThemeOptions &options() const noexcept;

private:
    UIThemeOptions _options;
    std::array<std::optional<UIColor>,
               static_cast<std::size_t>(UIColorRole::COUNT)>
        _colors{};
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UITheme;
using components::UIThemeOptions;

} // namespace engine::ecs

#endif /* !UITHEME_HPP_ */
