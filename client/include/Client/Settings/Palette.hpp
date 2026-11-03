/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Palette.hpp
*/

#ifndef PALETTE_HPP_
#define PALETTE_HPP_

#include <array>
#include <cstdint>
#include <string>

#include "Engine/Ecs/Components/UITheme.hpp"

namespace client::settings {

/** @brief Color sets, each made for one kind of color vision. */
enum class ColorPalette : std::uint8_t {
    NORMAL,
    PROTANOPIA,
    DEUTERANOPIA,
    TRITANOPIA
};

inline constexpr std::array<ColorPalette, 4> COLOR_PALETTES{
    ColorPalette::NORMAL, ColorPalette::PROTANOPIA, ColorPalette::DEUTERANOPIA,
    ColorPalette::TRITANOPIA};

/** @brief "Normal", "Protanopia"... */
[[nodiscard]] std::string paletteName(ColorPalette palette);

/** @brief "normal", "protanopia"..., as written in the settings file. */
[[nodiscard]] std::string paletteId(ColorPalette palette);

/**
 * @brief The theme of the chosen options.
 *
 * High contrast: pure colors, solid plates behind text, thicker focus and gauge
 * outlines; the player colors of the palette stay.
 */
[[nodiscard]] engine::ecs::UITheme
makeTheme(ColorPalette palette, bool highContrast, bool reducedMotion);

} // namespace client::settings

#endif /* !PALETTE_HPP_ */
