/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudView.cpp
*/

#include "Client/Hud/HudView.hpp"

#include <catch2/catch_test_macros.hpp>

#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

using client::hud::HudState;
using client::hud::HudView;
using client::hud::HudWidget;
using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::UILayoutSystem;
using engine::ecs::components::Rect;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIProgressBar;
using engine::ecs::components::UIText;
using engine::ecs::components::UITransform;

namespace {

Registry uiRegistry()
{
    Registry registry;

    engine::ecs::registerUIComponents(registry);
    return registry;
}

const std::string &textOf(const Registry &registry, Entity entity)
{
    return registry.getComponents<UIText>().get(entity)->text;
}

UIColor panelColor(const Registry &registry, Entity entity)
{
    return registry.getComponents<UIPanel>().get(entity)->color;
}

const UIProgressBar &barOf(const Registry &registry, Entity entity)
{
    return *registry.getComponents<UIProgressBar>().get(entity);
}

const Rect &boundsOf(const Registry &registry, Entity entity)
{
    return registry.getComponents<UITransform>().get(entity)->actualBounds;
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
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");
    HudState state = localPlayerState();
    state.lives = 2;
    state.level = 4;
    state.score = 79500;

    hud.sync(state);

    REQUIRE(textOf(registry, hud.widget(HudWidget::LIVES)) == "2");
    REQUIRE(textOf(registry, hud.widget(HudWidget::STAGE)) == "STAGE 4");
    REQUIRE(textOf(registry, hud.widget(HudWidget::SCORE)) == "0079500");
    REQUIRE(textOf(registry, hud.widget(HudWidget::BEAM_LABEL)) == "BEAM");
    REQUIRE(registry.getComponents<UIText>()
                .get(hud.widget(HudWidget::SCORE))
                ->fontId == "font.ttf");
}

TEST_CASE("the lives icon uses the local player's ship", "[client][hud][view]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");

    hud.sync(localPlayerState());

    const auto *icon = registry.getComponents<UIPanel>().get(
        hud.widget(HudWidget::LIVES_ICON));
    REQUIRE(icon->textureId == "sprites/r-typesheet42.gif");
    REQUIRE(icon->textureRect == Rect{66.0f, 17.0f, 33.0f, 17.0f});
}

TEST_CASE("the beam gauge follows the charge", "[client][hud][view]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");
    HudState state = localPlayerState();
    const Entity gauge = hud.widget(HudWidget::BEAM_GAUGE);

    state.beamCharge = 0.4f;
    hud.sync(state);
    REQUIRE(barOf(registry, gauge).value == 0.4f);
    const UIColor charging = barOf(registry, gauge).fillColor;

    state.beamCharge = 1.0f;
    hud.sync(state);
    REQUIRE(barOf(registry, gauge).fillColor != charging);
}

TEST_CASE("only the local tag is outlined", "[client][hud][view]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");

    hud.sync(localPlayerState());

    REQUIRE(panelColor(registry, hud.tagOutline(1)).a == 255);
    REQUIRE(panelColor(registry, hud.tagOutline(0)).a == 0);
    REQUIRE(panelColor(registry, hud.tag(0)) == HudView::playerColor(0));
    REQUIRE(panelColor(registry, hud.tag(2)) != HudView::playerColor(2));
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
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");
    const UILayoutSystem layout(1280.0f, 720.0f);

    hud.setMarker(1, Vec2{400.0f, 300.0f});
    hud.setMarker(0, Vec2{200.0f, 100.0f});
    hud.sync(localPlayerState());
    layout.update(registry);

    const Rect &local = boundsOf(registry, hud.marker(1));
    REQUIRE(local.x + local.width / 2.0f == 400.0f);
    REQUIRE(local.y + local.height < 300.0f);
    REQUIRE(textOf(registry, hud.marker(1)) == "P2");
    REQUIRE(panelColor(registry, hud.markerOutline(1)).a == 255);
    REQUIRE(panelColor(registry, hud.markerOutline(0)).a == 0);
}

TEST_CASE("markers hide without a ship or a player", "[client][hud][view]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");

    hud.setMarker(2, Vec2{400.0f, 300.0f});
    hud.setMarker(1, Vec2{400.0f, 300.0f});
    hud.setMarker(1, std::nullopt);
    hud.sync(localPlayerState());

    REQUIRE(textOf(registry, hud.marker(1)).empty());
    REQUIRE(panelColor(registry, hud.marker(1)).a == 0);
    REQUIRE(textOf(registry, hud.marker(2)).empty());
}

TEST_CASE("the bar stays at the bottom of any window", "[client][hud][view]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf", "ship.png", 1280.0f);
    UILayoutSystem layout(1280.0f, 720.0f);

    hud.resize(1920.0f);
    layout.setResolution(1920.0f, 1080.0f);
    layout.update(registry);

    const Rect &bar = boundsOf(registry, hud.widget(HudWidget::BAR));
    const Rect &score = boundsOf(registry, hud.widget(HudWidget::SCORE));
    REQUIRE(bar == Rect{0.0f, 1032.0f, 1920.0f, 48.0f});
    REQUIRE(score.x + score.width == 1904.0f);
    REQUIRE(score.y >= bar.y);
    REQUIRE(score.y + score.height <= 1080.0f);
}

TEST_CASE("the view lives in the game registry and cleans up after itself",
          "[client][hud][view]")
{
    Registry registry = uiRegistry();
    const Entity gameEntity = registry.spawnEntity();
    {
        const HudView hud(registry, "font.ttf");
        REQUIRE(registry.aliveCount() > 1);
        REQUIRE(registry.isAlive(hud.widget(HudWidget::SCORE)));
    }
    REQUIRE(registry.aliveCount() == 1);
    REQUIRE(registry.isAlive(gameEntity));
}

TEST_CASE("each player has its own shape, in the bar and above the ship",
          "[client][hud][view][accessibility]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");
    const auto &shapes = registry.getComponents<engine::ecs::UIShape>();

    hud.setMarker(1, Vec2{400.0f, 300.0f});
    hud.sync(localPlayerState());

    for (std::size_t i = 0; i < 4; ++i) {
        REQUIRE(shapes.get(hud.tagShape(i))->kind == HudView::playerShape(i));
        for (std::size_t j = i + 1; j < 4; ++j)
            REQUIRE(HudView::playerShape(i) != HudView::playerShape(j));
    }
    REQUIRE(shapes.get(hud.markerShape(1))->colorRole ==
            HudView::playerRole(1));
    REQUIRE(shapes.get(hud.markerShape(0))->color.a == 0);
}

TEST_CASE("the HUD colors go through the theme",
          "[client][hud][view][accessibility]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");

    hud.sync(localPlayerState());

    REQUIRE(registry.getComponents<UIPanel>().get(hud.tag(0))->colorRole ==
            engine::ecs::UIColorRole::PLAYER_1);
    REQUIRE(registry.getComponents<UIPanel>().get(hud.tag(2))->colorRole ==
            engine::ecs::UIColorRole::NONE);
    REQUIRE(barOf(registry, hud.widget(HudWidget::BEAM_GAUGE)).fillRole ==
            engine::ecs::UIColorRole::GAUGE);
}

TEST_CASE("reduced motion removes the flash of a full beam",
          "[client][hud][view][accessibility]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");
    HudState state = localPlayerState();
    state.beamCharge = 1.0f;

    hud.sync(state);
    REQUIRE(barOf(registry, hud.widget(HudWidget::BEAM_GAUGE)).fillRole ==
            engine::ecs::UIColorRole::GAUGE_FULL);

    state.reducedMotion = true;
    hud.sync(state);
    REQUIRE(barOf(registry, hud.widget(HudWidget::BEAM_GAUGE)).fillRole ==
            engine::ecs::UIColorRole::GAUGE);
}

TEST_CASE("the bar has no overlap at the largest UI scale",
          "[client][hud][view][accessibility]")
{
    Registry registry = uiRegistry();
    HudView hud(registry, "font.ttf");
    UILayoutSystem layout(1280.0f, 720.0f);

    layout.setScale(1.5f);
    layout.update(registry);

    const auto right = [&registry](Entity entity) {
        const Rect &box = boundsOf(registry, entity);
        return box.x + box.width;
    };
    const auto left = [&registry](Entity entity) {
        return boundsOf(registry, entity).x;
    };
    REQUIRE(right(hud.tagShape(3)) <= left(hud.widget(HudWidget::LIVES_ICON)));
    REQUIRE(right(hud.widget(HudWidget::LIVES)) <=
            left(hud.widget(HudWidget::STAGE)));
    REQUIRE(right(hud.widget(HudWidget::STAGE)) <=
            left(hud.widget(HudWidget::BEAM_LABEL)));
    REQUIRE(right(hud.widget(HudWidget::BEAM_LABEL)) <=
            left(hud.widget(HudWidget::BEAM_FRAME)));
    REQUIRE(right(hud.widget(HudWidget::BEAM_FRAME)) <=
            left(hud.widget(HudWidget::SCORE)));
    REQUIRE(right(hud.widget(HudWidget::SCORE)) <= 1280.0f);
}
