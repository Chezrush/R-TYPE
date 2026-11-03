/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UITheme.cpp
*/

#include "Engine/Ecs/Components/UITheme.hpp"

#include <catch2/catch_test_macros.hpp>

using engine::ecs::UIColorRole;
using engine::ecs::UITheme;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIWidgetStyle;

namespace {

constexpr UIColor OWN{1, 2, 3, 255};
constexpr UIColor THEMED{9, 8, 7, 255};

} // namespace

TEST_CASE("an empty theme keeps the colors of the components",
          "[ecs][ui][theme]")
{
    const UITheme theme;

    REQUIRE(theme.resolve(UIColorRole::TEXT, OWN) == OWN);
    REQUIRE(theme.resolve(UIColorRole::NONE, OWN) == OWN);
    REQUIRE(theme.styled(UIWidgetStyle{}) == UIWidgetStyle{});
}

TEST_CASE("a role takes the color of the theme", "[ecs][ui][theme]")
{
    UITheme theme;

    theme.setColor(UIColorRole::GAUGE, THEMED);

    REQUIRE(theme.resolve(UIColorRole::GAUGE, OWN) == THEMED);
    REQUIRE(theme.resolve(UIColorRole::ACCENT, OWN) == OWN);
}

TEST_CASE("NONE and COUNT never take a color", "[ecs][ui][theme]")
{
    UITheme theme;

    theme.setColor(UIColorRole::NONE, THEMED);
    theme.setColor(UIColorRole::COUNT, THEMED);

    REQUIRE(theme.resolve(UIColorRole::NONE, OWN) == OWN);
    REQUIRE(theme.resolve(UIColorRole::COUNT, OWN) == OWN);
}

TEST_CASE("widget styles follow the theme", "[ecs][ui][theme]")
{
    UITheme theme;
    UIWidgetStyle style;
    style.fontId = "font.ttf";

    theme.setColor(UIColorRole::SURFACE, THEMED);
    theme.setColor(UIColorRole::FOCUS, OWN);
    const UIWidgetStyle styled = theme.styled(style);

    REQUIRE(styled.background == THEMED);
    REQUIRE(styled.focusOutline == OWN);
    REQUIRE(styled.foreground == style.foreground);
    REQUIRE(styled.fontId == "font.ttf");
}

TEST_CASE("text on a player color gets no plate", "[ecs][ui][theme]")
{
    UITheme theme;

    REQUIRE_FALSE(theme.hasPlate(UIColorRole::TEXT));
    theme.options().textPlates = true;
    REQUIRE(theme.hasPlate(UIColorRole::TEXT));
    REQUIRE(theme.hasPlate(UIColorRole::NONE));
    REQUIRE_FALSE(theme.hasPlate(UIColorRole::TEXT_ON_PLAYER));
}
