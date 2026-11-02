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
using engine::core::OverlayCloseRequested;
using engine::core::OverlayRequested;
using engine::core::QuitRequested;
using engine::core::SceneManager;
using engine::core::SceneRouter;

namespace {

enum class Screen : std::uint8_t {
    TITLE,
    SHOP,
    POPUP,
    UNKNOWN
};

class NamedScene : public IScene {
public:
    explicit NamedScene(std::string sceneName, bool isOverlayScene = false)
        : name(std::move(sceneName)),
          overlay(isOverlayScene)
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
        return overlay;
    }

    std::string name;
    bool overlay;
};

struct RouterFixture {
    RouterFixture()
    {
        router.setRoute(Screen::TITLE,
                        [] { return std::make_unique<NamedScene>("title"); });
        router.setRoute(Screen::SHOP,
                        [] { return std::make_unique<NamedScene>("shop"); });
        router.setRoute(Screen::POPUP, [] {
            return std::make_unique<NamedScene>("popup", true);
        });
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

TEST_CASE("overlays are pushed and closed when applied",
          "[core][scene][router]")
{
    RouterFixture fixture;
    fixture.go(Screen::TITLE);
    fixture.router.apply();

    fixture.bus.publish(OverlayRequested<Screen>{Screen::POPUP});
    REQUIRE(fixture.scenes.size() == 1);
    fixture.router.apply();
    REQUIRE(fixture.current() == "popup");

    fixture.bus.publish(OverlayCloseRequested{});
    REQUIRE(fixture.scenes.size() == 2);
    fixture.router.apply();
    REQUIRE(fixture.current() == "title");
}

TEST_CASE("closing never removes a scene that is not an overlay",
          "[core][scene][router]")
{
    RouterFixture fixture;

    fixture.bus.publish(OverlayCloseRequested{});
    fixture.router.apply();
    fixture.go(Screen::TITLE);
    fixture.router.apply();
    fixture.bus.publish(OverlayCloseRequested{});
    fixture.router.apply();

    REQUIRE(fixture.current() == "title");
}

TEST_CASE("an overlay replacing another one keeps the order",
          "[core][scene][router]")
{
    RouterFixture fixture;
    fixture.go(Screen::TITLE);
    fixture.router.apply();
    fixture.bus.publish(OverlayRequested<Screen>{Screen::POPUP});
    fixture.router.apply();

    fixture.bus.publish(OverlayCloseRequested{});
    fixture.bus.publish(OverlayRequested<Screen>{Screen::POPUP});
    fixture.router.apply();

    REQUIRE(fixture.scenes.size() == 2);
}

TEST_CASE("a navigation replaces the requests before it",
          "[core][scene][router]")
{
    RouterFixture fixture;
    fixture.go(Screen::TITLE);
    fixture.router.apply();

    fixture.bus.publish(OverlayRequested<Screen>{Screen::POPUP});
    fixture.go(Screen::SHOP);
    fixture.router.apply();

    REQUIRE(fixture.current() == "shop");
    REQUIRE(fixture.scenes.size() == 1);
}

TEST_CASE("an unknown route neither pushes nor clears the stack",
          "[core][scene][router]")
{
    RouterFixture fixture;
    fixture.go(Screen::TITLE);
    fixture.router.apply();

    fixture.bus.publish(OverlayRequested<Screen>{Screen::UNKNOWN});
    fixture.router.apply();

    REQUIRE(fixture.current() == "title");
    REQUIRE(fixture.scenes.size() == 1);
}

TEST_CASE("clearing closes every overlay and nothing else",
          "[core][scene][router]")
{
    RouterFixture fixture;
    fixture.go(Screen::TITLE);
    fixture.router.apply();
    fixture.bus.publish(OverlayRequested<Screen>{Screen::POPUP});
    fixture.bus.publish(OverlayRequested<Screen>{Screen::POPUP});
    fixture.router.apply();
    REQUIRE(fixture.scenes.size() == 3);

    fixture.bus.publish(engine::core::OverlaysClearRequested{});
    fixture.router.apply();

    REQUIRE(fixture.current() == "title");
    REQUIRE(fixture.scenes.size() == 1);
}
