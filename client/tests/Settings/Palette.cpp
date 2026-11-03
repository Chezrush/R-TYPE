/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Palette.cpp
*/

#include "Client/Settings/Palette.hpp"

#include <algorithm>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>

#include "Client/Settings/ClientSettings.hpp"

using client::settings::COLOR_PALETTES;
using client::settings::ColorPalette;
using client::settings::makeTheme;
using engine::ecs::UIColorRole;
using engine::ecs::components::UIColor;

namespace {

using Matrix = std::array<std::array<double, 3>, 3>;
using Vector = std::array<double, 3>;

constexpr std::array<UIColorRole, 4> PLAYERS{
    UIColorRole::PLAYER_1, UIColorRole::PLAYER_2, UIColorRole::PLAYER_3,
    UIColorRole::PLAYER_4};

// Machado, Oliveira and Fernandes (2009), severity 1.0, on linear RGB.
constexpr Matrix NORMAL_VISION{
    {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};
constexpr Matrix PROTANOPIA_VISION{{{0.152286, 1.052583, -0.204868},
                                    {0.114503, 0.786281, 0.099216},
                                    {-0.003882, -0.048116, 1.051998}}};
constexpr Matrix DEUTERANOPIA_VISION{{{0.367322, 0.860646, -0.227968},
                                      {0.280085, 0.672501, 0.047413},
                                      {-0.011820, 0.042940, 0.968881}}};
constexpr Matrix TRITANOPIA_VISION{{{1.255528, -0.076749, -0.178779},
                                    {-0.078411, 0.930809, 0.147602},
                                    {0.004733, 0.691367, 0.303900}}};
constexpr Matrix RGB_TO_XYZ{{{0.4124, 0.3576, 0.1805},
                             {0.2126, 0.7152, 0.0722},
                             {0.0193, 0.1192, 0.9505}}};
constexpr Vector WHITE_POINT{0.95047, 1.0, 1.08883};

/** @brief Below this distance (CIE76), two colors are easy to mix up. */
constexpr double MIN_COLOR_DISTANCE = 25.0;

const Matrix &visionOf(ColorPalette palette)
{
    switch (palette) {
        case ColorPalette::PROTANOPIA:
            return PROTANOPIA_VISION;
        case ColorPalette::DEUTERANOPIA:
            return DEUTERANOPIA_VISION;
        case ColorPalette::TRITANOPIA:
            return TRITANOPIA_VISION;
        default:
            return NORMAL_VISION;
    }
}

Vector multiply(const Matrix &matrix, const Vector &vector)
{
    Vector result{};

    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t col = 0; col < 3; ++col)
            result.at(row) += matrix.at(row).at(col) * vector.at(col);
    }
    return result;
}

double toLinear(std::uint8_t channel)
{
    const double value = channel / 255.0;

    return value <= 0.04045 ? value / 12.92
                            : std::pow((value + 0.055) / 1.055, 2.4);
}

double labCurve(double value)
{
    return value > 0.008856 ? std::cbrt(value) : 7.787 * value + 16.0 / 116.0;
}

Vector seenAsLab(UIColor color, const Matrix &vision)
{
    Vector rgb = multiply(
        vision, {toLinear(color.r), toLinear(color.g), toLinear(color.b)});

    for (double &channel : rgb)
        channel = std::clamp(channel, 0.0, 1.0);
    const Vector xyz = multiply(RGB_TO_XYZ, rgb);
    const double x = labCurve(xyz[0] / WHITE_POINT[0]);
    const double y = labCurve(xyz[1] / WHITE_POINT[1]);
    const double z = labCurve(xyz[2] / WHITE_POINT[2]);

    return {116.0 * y - 16.0, 500.0 * (x - y), 200.0 * (y - z)};
}

double distance(const Vector &first, const Vector &second)
{
    return std::hypot(first[0] - second[0], first[1] - second[1],
                      first[2] - second[2]);
}

double luminance(UIColor color)
{
    return 0.2126 * toLinear(color.r) + 0.7152 * toLinear(color.g) +
           0.0722 * toLinear(color.b);
}

double contrast(UIColor first, UIColor second)
{
    const double a = luminance(first) + 0.05;
    const double b = luminance(second) + 0.05;

    return std::max(a, b) / std::min(a, b);
}

UIColor colorOf(ColorPalette palette, UIColorRole role)
{
    return makeTheme(palette, false, false).resolve(role, UIColor{1, 2, 3, 4});
}

} // namespace

TEST_CASE("players stay distinct for the vision each palette targets",
          "[client][settings][palette]")
{
    for (const ColorPalette palette : COLOR_PALETTES) {
        const Matrix &vision = visionOf(palette);

        for (std::size_t i = 0; i < PLAYERS.size(); ++i) {
            for (std::size_t j = i + 1; j < PLAYERS.size(); ++j) {
                INFO(client::settings::paletteName(palette)
                     << " players " << i << " and " << j);
                REQUIRE(
                    distance(seenAsLab(colorOf(palette, PLAYERS.at(i)), vision),
                             seenAsLab(colorOf(palette, PLAYERS.at(j)),
                                       vision)) >= MIN_COLOR_DISTANCE);
            }
        }
    }
}

TEST_CASE("the text on a player tag stays readable",
          "[client][settings][palette]")
{
    for (const ColorPalette palette : COLOR_PALETTES) {
        for (const UIColorRole player : PLAYERS) {
            INFO(client::settings::paletteName(palette));
            REQUIRE(contrast(colorOf(palette, player),
                             colorOf(palette, UIColorRole::TEXT_ON_PLAYER)) >=
                    4.5);
        }
    }
}

TEST_CASE("high contrast adds plates and thick outlines",
          "[client][settings][palette]")
{
    const auto normal = makeTheme(ColorPalette::NORMAL, false, false);
    const auto strong = makeTheme(ColorPalette::NORMAL, true, false);
    const UIColor unused{};

    REQUIRE_FALSE(normal.options().textPlates);
    REQUIRE(strong.options().textPlates);
    REQUIRE(strong.options().focusThickness > normal.options().focusThickness);
    REQUIRE(strong.options().gaugeOutline > normal.options().gaugeOutline);
    REQUIRE(strong.resolve(UIColorRole::PLAYER_1, unused) ==
            normal.resolve(UIColorRole::PLAYER_1, unused));
}

TEST_CASE("the accessibility options survive the settings file",
          "[client][settings][palette]")
{
    client::settings::ClientSettings settings;
    settings.palette = ColorPalette::TRITANOPIA;
    settings.highContrast = true;
    settings.reducedMotion = true;

    REQUIRE(client::settings::parseSettings(
                client::settings::formatSettings(settings)) == settings);
    REQUIRE(
        client::settings::parseSettings("palette=sepia\nhigh_contrast=2\n") ==
        client::settings::ClientSettings{});
}
