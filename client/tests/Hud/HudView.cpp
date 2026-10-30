/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudView.cpp
*/

#include "Client/Hud/HudView.hpp"

#include <catch2/catch_test_macros.hpp>

using client::hud::HudState;
using client::hud::HudView;
using client::hud::HudWidget;
using engine::ecs::Entity;
using engine::ecs::components::Rect;
using engine::ecs::components::UIColor;
using engine::ecs::components::Vec2;

namespace {

const std::string &textOf(const HudView &hud, Entity entity)
{
    return hud.texts().get(entity)->text;
}

UIColor panelColor(const HudView &hud, Entity entity)
{
    return hud.panels().get(entity)->color;
}

const Rect &boundsOf(const HudView &hud, Entity entity)
{
    return hud.transforms().get(entity)->actualBounds;
}

HudState localPlayerState()
{
    HudState state;

    state.localPlayer = 1;
    state.localAlive = true;
    state.connected = {true, true, false, false};
    return state;
}

} // namespace

TEST_CASE("score is shown with 7 digits", "[client][hud][view]")
{
    REQUIRE(HudView::formatScore(0) == "0000000");
    REQUIRE(HudView::formatScore(79500) == "0079500");
    REQUIRE(HudView::formatScore(123456789) == "9999999");
}

TEST_CASE("player tags are P1 to P4", "[client][hud][view]")
{
    REQUIRE(HudView::playerTag(0) == "P1");
    REQUIRE(HudView::playerTag(3) == "P4");
}

TEST_CASE("the bar shows lives, stage and score", "[client][hud][view]")
{
    HudView hud("font.ttf");
    HudState state = localPlayerState();
    state.lives = 2;
    state.level = 4;
    state.score = 79500;

    hud.sync(state);

    REQUIRE(textOf(hud, hud.widget(HudWidget::LIVES)) == "2");
    REQUIRE(textOf(hud, hud.widget(HudWidget::STAGE)) == "STAGE 4");
    REQUIRE(textOf(hud, hud.widget(HudWidget::SCORE)) == "0079500");
    REQUIRE(textOf(hud, hud.widget(HudWidget::BEAM_LABEL)) == "BEAM");
    REQUIRE(hud.texts().get(hud.widget(HudWidget::SCORE))->fontId ==
            "font.ttf");
}

TEST_CASE("the lives icon uses the local player's ship", "[client][hud][view]")
{
    HudView hud("font.ttf");

    hud.sync(localPlayerState());

    const auto *icon = hud.panels().get(hud.widget(HudWidget::LIVES_ICON));
    REQUIRE(icon->textureId == "sprites/r-typesheet42.gif");
    REQUIRE(icon->textureRect == Rect{66.0f, 17.0f, 33.0f, 17.0f});
}

TEST_CASE("the beam gauge follows the charge", "[client][hud][view]")
{
    HudView hud("font.ttf");
    HudState state = localPlayerState();
    const Entity gauge = hud.widget(HudWidget::BEAM_GAUGE);

    state.beamCharge = 0.4f;
    hud.sync(state);
    REQUIRE(hud.bars().get(gauge)->value == 0.4f);
    const UIColor charging = hud.bars().get(gauge)->fillColor;

    state.beamCharge = 1.0f;
    hud.sync(state);
    REQUIRE(hud.bars().get(gauge)->fillColor != charging);
}

TEST_CASE("only the local tag is outlined", "[client][hud][view]")
{
    HudView hud("font.ttf");

    hud.sync(localPlayerState());

    REQUIRE(panelColor(hud, hud.tagOutline(1)).a == 255);
    REQUIRE(panelColor(hud, hud.tagOutline(0)).a == 0);
    REQUIRE(panelColor(hud, hud.tag(0)) == HudView::playerColor(0));
    REQUIRE(panelColor(hud, hud.tag(2)) != HudView::playerColor(2));
}

TEST_CASE("player colors are all different", "[client][hud][view]")
{
    for (std::size_t i = 0; i < 4; ++i) {
        for (std::size_t j = i + 1; j < 4; ++j)
            REQUIRE(HudView::playerColor(i) != HudView::playerColor(j));
    }
}

TEST_CASE("markers follow ships and outline the local one",
          "[client][hud][view]")
{
    HudView hud("font.ttf");

    hud.setMarker(1, Vec2{400.0f, 300.0f});
    hud.setMarker(0, Vec2{200.0f, 100.0f});
    hud.sync(localPlayerState());

    const Rect &local = boundsOf(hud, hud.marker(1));
    REQUIRE(local.x + local.width / 2.0f == 400.0f);
    REQUIRE(local.y + local.height < 300.0f);
    REQUIRE(textOf(hud, hud.marker(1)) == "P2");
    REQUIRE(panelColor(hud, hud.markerOutline(1)).a == 255);
    REQUIRE(panelColor(hud, hud.markerOutline(0)).a == 0);
}

TEST_CASE("markers hide without a ship or a player", "[client][hud][view]")
{
    HudView hud("font.ttf");

    hud.setMarker(2, Vec2{400.0f, 300.0f});
    hud.setMarker(1, Vec2{400.0f, 300.0f});
    hud.setMarker(1, std::nullopt);
    hud.sync(localPlayerState());

    REQUIRE(textOf(hud, hud.marker(1)).empty());
    REQUIRE(panelColor(hud, hud.marker(1)).a == 0);
    REQUIRE(textOf(hud, hud.marker(2)).empty());
}

TEST_CASE("the bar stays at the bottom of any window", "[client][hud][view]")
{
    HudView hud("font.ttf", "ship.png", 1280.0f, 720.0f);

    hud.resize(1920.0f, 1080.0f);

    const Rect &bar = boundsOf(hud, hud.widget(HudWidget::BAR));
    const Rect &score = boundsOf(hud, hud.widget(HudWidget::SCORE));
    REQUIRE(bar == Rect{0.0f, 1032.0f, 1920.0f, 48.0f});
    REQUIRE(score.x + score.width == 1904.0f);
    REQUIRE(score.y >= bar.y);
    REQUIRE(score.y + score.height <= 1080.0f);
}
