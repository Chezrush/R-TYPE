/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneRouter.cpp
*/

#include "Engine/Core/SceneRouter.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

using engine::core::EventBus;
using engine::core::IScene;
using engine::core::NavigationRequested;
using engine::core::QuitRequested;
using engine::core::SceneManager;
using engine::core::SceneRouter;

namespace {

enum class Screen : std::uint8_t {
    TITLE,
    SHOP,
    UNKNOWN
};

class NamedScene : public IScene {
public:
    explicit NamedScene(std::string name)
        : name(std::move(name))
    {}

    void onEnter() override {}

    void onExit() override {}

    void onPause() override {}

    void onResume() override {}

    void update(float /*dt*/) override {}

    void render() override {}

    void handleInput() override {}

    [[nodiscard]] bool isOverlay() const noexcept override
    {
        return false;
    }

    std::string name;
};

struct RouterFixture {
    RouterFixture()
    {
        router.setRoute(Screen::TITLE,
                        [] { return std::make_unique<NamedScene>("title"); });
        router.setRoute(Screen::SHOP,
                        [] { return std::make_unique<NamedScene>("shop"); });
    }

    void go(Screen screen)
    {
        bus.publish(NavigationRequested<Screen>{screen});
    }

    [[nodiscard]] const std::string &current()
    {
        return dynamic_cast<NamedScene &>(scenes.currentScene()).name;
    }

    SceneManager scenes;
    EventBus bus;
    SceneRouter<Screen> router{scenes, bus};
};

} // namespace

TEST_CASE("the router only switches when applied", "[core][scene][router]")
{
    RouterFixture fixture;

    fixture.go(Screen::SHOP);
    REQUIRE(fixture.scenes.empty());

    fixture.router.apply();
    REQUIRE(fixture.current() == "shop");
    REQUIRE(fixture.scenes.size() == 1);
}

TEST_CASE("a request is only applied once", "[core][scene][router]")
{
    RouterFixture fixture;

    fixture.go(Screen::SHOP);
    fixture.router.apply();
    fixture.scenes.switchScene(std::make_unique<NamedScene>("other"));
    fixture.router.apply();

    REQUIRE(fixture.current() == "other");
}

TEST_CASE("a route without a scene is ignored", "[core][scene][router]")
{
    RouterFixture fixture;

    fixture.go(Screen::TITLE);
    fixture.router.apply();
    fixture.go(Screen::UNKNOWN);
    fixture.router.apply();

    REQUIRE(fixture.current() == "title");
    REQUIRE_FALSE(fixture.router.quitRequested());
}

TEST_CASE("the last request of a frame wins", "[core][scene][router]")
{
    RouterFixture fixture;

    fixture.go(Screen::SHOP);
    fixture.go(Screen::TITLE);
    fixture.router.apply();

    REQUIRE(fixture.current() == "title");
}

TEST_CASE("the router reports a quit", "[core][scene][router]")
{
    RouterFixture fixture;

    REQUIRE_FALSE(fixture.router.quitRequested());
    fixture.bus.publish(QuitRequested{});
    REQUIRE(fixture.router.quitRequested());
}

TEST_CASE("routes can be any ordered type", "[core][scene][router]")
{
    SceneManager scenes;
    EventBus bus;
    SceneRouter<std::string> router(scenes, bus);

    router.setRoute("title",
                    [] { return std::make_unique<NamedScene>("title"); });
    bus.publish(NavigationRequested<std::string>{"title"});
    router.apply();

    REQUIRE(scenes.size() == 1);
}

TEST_CASE("a destroyed router stops listening", "[core][scene][router]")
{
    SceneManager scenes;
    EventBus bus;
    {
        const SceneRouter<Screen> router(scenes, bus);
    }

    bus.publish(NavigationRequested<Screen>{Screen::TITLE});
    bus.publish(QuitRequested{});

    REQUIRE(scenes.empty());
}
