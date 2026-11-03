/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Palette.cpp
*/

#include "Client/Settings/Palette.hpp"

#include <cstddef>

namespace client::settings {

namespace {

using engine::ecs::UIColorRole;
using engine::ecs::UITheme;
using engine::ecs::components::UIColor;

struct PaletteColors {
    const char *id;
    const char *name;
    std::array<UIColor, 4> players;
    UIColor accent;
    UIColor gauge;
    UIColor error;
    UIColor warning;
};

constexpr std::array<PaletteColors, COLOR_PALETTES.size()> PALETTES{{
    {"normal",
     "Normal",
     {{{0, 190, 230, 255},
       {210, 70, 210, 255},
       {40, 200, 70, 255},
       {230, 70, 60, 255}}},
     {120, 180, 255, 255},
     {110, 190, 255, 255},
     {255, 110, 100, 255},
     {255, 190, 80, 255}},
    {"protanopia",
     "Protanopia",
     {{{240, 228, 66, 255},
       {0, 136, 204, 255},
       {240, 240, 240, 255},
       {150, 110, 0, 255}}},
     {86, 180, 233, 255},
     {86, 180, 233, 255},
     {240, 228, 66, 255},
     {240, 240, 240, 255}},
    {"deuteranopia",
     "Deuteranopia",
     {{{230, 159, 0, 255},
       {0, 136, 204, 255},
       {240, 240, 240, 255},
       {120, 120, 140, 255}}},
     {86, 180, 233, 255},
     {86, 180, 233, 255},
     {230, 159, 0, 255},
     {240, 228, 66, 255}},
    {"tritanopia",
     "Tritanopia",
     {{{213, 94, 0, 255},
       {0, 220, 220, 255},
       {240, 240, 240, 255},
       {204, 121, 167, 255}}},
     {0, 220, 220, 255},
     {0, 220, 220, 255},
     {255, 90, 90, 255},
     {255, 170, 170, 255}},
}};

constexpr UIColor TEXT{235, 235, 255, 255};
constexpr UIColor TEXT_MUTED{130, 130, 145, 255};
constexpr UIColor ON_PLAYER{0, 0, 0, 255};
constexpr UIColor BLACK{0, 0, 0, 255};
constexpr UIColor WHITE{255, 255, 255, 255};
constexpr UIColor SURFACE{30, 30, 40, 230};
constexpr UIColor BUTTON{24, 36, 90, 255};
constexpr UIColor BUTTON_HOVERED{50, 80, 190, 255};
constexpr UIColor BUTTON_PRESSED{16, 24, 60, 255};
constexpr UIColor BUTTON_DISABLED{40, 40, 50, 180};
constexpr UIColor FOCUS{255, 215, 0, 255};
constexpr UIColor STRONG_MUTED{200, 200, 200, 255};
constexpr UIColor STRONG_BUTTON{0, 0, 0, 255};
constexpr UIColor STRONG_HOVERED{70, 70, 70, 255};
constexpr float STRONG_FOCUS_THICKNESS = 4.0f;
constexpr float STRONG_GAUGE_OUTLINE = 2.0f;

const PaletteColors &colorsOf(ColorPalette palette)
{
    return PALETTES.at(static_cast<std::size_t>(palette));
}

void setBaseColors(UITheme &theme)
{
    theme.setColor(UIColorRole::TEXT, TEXT);
    theme.setColor(UIColorRole::TEXT_MUTED, TEXT_MUTED);
    theme.setColor(UIColorRole::TEXT_ON_PLAYER, ON_PLAYER);
    theme.setColor(UIColorRole::BACKGROUND, BLACK);
    theme.setColor(UIColorRole::SURFACE, SURFACE);
    theme.setColor(UIColorRole::BUTTON, BUTTON);
    theme.setColor(UIColorRole::BUTTON_HOVERED, BUTTON_HOVERED);
    theme.setColor(UIColorRole::BUTTON_PRESSED, BUTTON_PRESSED);
    theme.setColor(UIColorRole::BUTTON_DISABLED, BUTTON_DISABLED);
    theme.setColor(UIColorRole::FOCUS, FOCUS);
    theme.setColor(UIColorRole::GAUGE_FULL, WHITE);
}

void setPaletteColors(UITheme &theme, const PaletteColors &colors)
{
    theme.setColor(UIColorRole::ACCENT, colors.accent);
    theme.setColor(UIColorRole::GAUGE, colors.gauge);
    theme.setColor(UIColorRole::ERROR_TEXT, colors.error);
    theme.setColor(UIColorRole::WARNING, colors.warning);
    theme.setColor(UIColorRole::PLAYER_1, colors.players[0]);
    theme.setColor(UIColorRole::PLAYER_2, colors.players[1]);
    theme.setColor(UIColorRole::PLAYER_3, colors.players[2]);
    theme.setColor(UIColorRole::PLAYER_4, colors.players[3]);
}

void setHighContrast(UITheme &theme)
{
    theme.setColor(UIColorRole::TEXT, WHITE);
    theme.setColor(UIColorRole::TEXT_MUTED, STRONG_MUTED);
    theme.setColor(UIColorRole::SURFACE, BLACK);
    theme.setColor(UIColorRole::BUTTON, STRONG_BUTTON);
    theme.setColor(UIColorRole::BUTTON_HOVERED, STRONG_HOVERED);
    theme.options().textPlates = true;
    theme.options().focusThickness = STRONG_FOCUS_THICKNESS;
    theme.options().gaugeOutline = STRONG_GAUGE_OUTLINE;
}

} // namespace

std::string paletteName(ColorPalette palette)
{
    return colorsOf(palette).name;
}

std::string paletteId(ColorPalette palette)
{
    return colorsOf(palette).id;
}

UITheme makeTheme(ColorPalette palette, bool highContrast, bool reducedMotion)
{
    UITheme theme;

    setBaseColors(theme);
    setPaletteColors(theme, colorsOf(palette));
    if (highContrast)
        setHighContrast(theme);
    theme.options().reducedMotion = reducedMotion;
    return theme;
}

} // namespace client::settings
