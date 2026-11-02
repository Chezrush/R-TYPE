/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MainMenuScene.cpp
*/

#include "Client/Scenes/MainMenuScene.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>

#include "Client/Scenes/Starfield.hpp"
#include "SceneFixture.hpp"

using client::scenes::MainMenuScene;
using client::scenes::SceneRoute;
using client::scenes::Starfield;
using engine::ecs::UIInteractionSystem;
using engine::ecs::UIKey;
using engine::ecs::components::UIText;

namespace {

MainMenuScene &enterMenu(SceneFixture &fixture)
{
    auto scene = std::make_unique<MainMenuScene>(fixture.context());
    MainMenuScene &menu = *scene;

    fixture.scenes.pushScene(std::move(scene));
    fixture.frame();
    return menu;
}

} // namespace

TEST_CASE("each menu button asks for its screen", "[client][scenes][menu]")
{
    const std::pair<const char *, SceneRoute> entries[] = {
        {"Play", SceneRoute::CONNECT},
        {"Settings", SceneRoute::SETTINGS},
        {"Controls", SceneRoute::CONTROLS},
        {"How to play", SceneRoute::HOW_TO_PLAY},
    };

    for (const auto &[label, route] : entries) {
        SceneFixture fixture;
        const MainMenuScene &menu = enterMenu(fixture);

        fixture.click(menu.button(label));

        REQUIRE(fixture.navigations == 1);
        REQUIRE(fixture.route == route);
        REQUIRE(fixture.registry.getComponents<UIText>()
                    .get(menu.button(label))
                    ->text == label);
    }
}

TEST_CASE("Quit asks to quit", "[client][scenes][menu]")
{
    SceneFixture fixture;
    const MainMenuScene &menu = enterMenu(fixture);

    fixture.click(menu.button("Quit"));

    REQUIRE(fixture.quits == 1);
    REQUIRE(fixture.navigations == 0);
}

TEST_CASE("menu buttons use the R-Type colors", "[client][scenes][menu]")
{
    SceneFixture fixture;
    const MainMenuScene &menu = enterMenu(fixture);
    const auto *button =
        fixture.registry.getComponents<engine::ecs::UIButton>().get(
            menu.button("Play"));

    REQUIRE(button->normalColor != engine::ecs::UIButton{}.normalColor);
}

TEST_CASE("the menu works with the keyboard alone", "[client][scenes][menu]")
{
    SceneFixture fixture;
    const MainMenuScene &menu = enterMenu(fixture);

    REQUIRE(UIInteractionSystem::focusedEntity(fixture.registry) ==
            menu.button("Play"));
    fixture.key(UIKey::DOWN);
    REQUIRE(UIInteractionSystem::focusedEntity(fixture.registry) ==
            menu.button("Settings"));
    fixture.key(UIKey::UP);
    fixture.key(UIKey::ENTER);
    REQUIRE(fixture.route == SceneRoute::CONNECT);
}

TEST_CASE("the menu removes its widgets and listeners on exit",
          "[client][scenes][menu]")
{
    SceneFixture fixture;
    const MainMenuScene &menu = enterMenu(fixture);
    const auto play = menu.button("Play");

    REQUIRE(fixture.registry.aliveCount() > 0);
    fixture.scenes.popScene();
    REQUIRE(fixture.registry.aliveCount() == 0);

    fixture.bus.publish(
        engine::ecs::UIButtonClickedEvent{play.index(), "Play"});
    REQUIRE(fixture.navigations == 0);
}

TEST_CASE("a paused menu is rebuilt when it resumes", "[client][scenes][menu]")
{
    SceneFixture fixture;
    const MainMenuScene &menu = enterMenu(fixture);
    const auto widgets = fixture.registry.aliveCount();

    fixture.scenes.currentScene().onPause();
    REQUIRE(fixture.registry.aliveCount() == 0);
    fixture.scenes.currentScene().onResume();
    REQUIRE(fixture.registry.aliveCount() == widgets);
    fixture.click(menu.button("Play"));
    REQUIRE(fixture.route == SceneRoute::CONNECT);
}

TEST_CASE("stars scroll left and wrap around", "[client][scenes][menu]")
{
    Starfield field(50, 7);
    const Starfield same(50, 7);

    REQUIRE(field.stars().size() == 50);
    REQUIRE(field.stars()[0].position == same.stars()[0].position);
    const float before = field.stars()[0].position.x;

    field.advance(0.001f);
    REQUIRE(field.stars()[0].position.x != before);
    for (int i = 0; i < 1000; ++i) {
        field.advance(0.5f);
        for (const auto &star : field.stars()) {
            REQUIRE(star.position.x >= 0.0f);
            REQUIRE(star.position.x <= 1.0f);
        }
    }
}

TEST_CASE("the stars of the menu move on screen", "[client][scenes][menu]")
{
    SceneFixture fixture;
    const MainMenuScene &menu = enterMenu(fixture);
    const float x = menu.starfield().stars()[0].position.x;

    fixture.scenes.update(0.5f);

    REQUIRE(menu.starfield().stars()[0].position.x != x);
}
