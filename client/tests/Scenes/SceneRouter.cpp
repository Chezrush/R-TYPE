/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneRouter.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "Client/Scenes/ConnectScene.hpp"
#include "Client/Scenes/MainMenuScene.hpp"
#include "SceneFixture.hpp"

using client::scenes::ConnectScene;
using client::scenes::MainMenuScene;
using client::scenes::NavigationRequested;
using client::scenes::SceneRoute;
using client::scenes::SceneRouter;

namespace {

void addMenuRoutes(SceneRouter &router, SceneFixture &fixture)
{
    router.setRoute(SceneRoute::MAIN_MENU, [&fixture] {
        return std::make_unique<MainMenuScene>(fixture.context());
    });
    router.setRoute(SceneRoute::CONNECT, [&fixture] {
        return std::make_unique<ConnectScene>(fixture.context());
    });
}

} // namespace

TEST_CASE("a click on Play opens the connection screen",
          "[client][scenes][router]")
{
    SceneFixture fixture;
    SceneRouter router(fixture.scenes, fixture.bus);
    addMenuRoutes(router, fixture);
    fixture.bus.publish(NavigationRequested{SceneRoute::MAIN_MENU});
    router.apply();
    const auto &menu =
        dynamic_cast<const MainMenuScene &>(fixture.scenes.currentScene());

    fixture.click(menu.button("Play"));
    router.apply();
    REQUIRE(dynamic_cast<ConnectScene *>(&fixture.scenes.currentScene()) !=
            nullptr);

    fixture.key(engine::ecs::UIKey::ESCAPE);
    router.apply();
    REQUIRE(dynamic_cast<MainMenuScene *>(&fixture.scenes.currentScene()) !=
            nullptr);
    REQUIRE(fixture.scenes.size() == 1);
}

TEST_CASE("a click on Quit reaches the router", "[client][scenes][router]")
{
    SceneFixture fixture;
    SceneRouter router(fixture.scenes, fixture.bus);
    addMenuRoutes(router, fixture);
    fixture.bus.publish(NavigationRequested{SceneRoute::MAIN_MENU});
    router.apply();
    const auto &menu =
        dynamic_cast<const MainMenuScene &>(fixture.scenes.currentScene());

    fixture.click(menu.button("Quit"));

    REQUIRE(router.quitRequested());
}
