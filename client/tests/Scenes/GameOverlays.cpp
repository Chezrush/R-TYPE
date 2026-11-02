/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameOverlays.cpp
*/

#include "Client/Scenes/GameOverlays.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>

#include "Client/Scenes/GameOverOverlay.hpp"
#include "Client/Scenes/InGameMenuOverlay.hpp"
#include "Client/Scenes/MainMenuScene.hpp"
#include "Client/Scenes/RespawnOverlay.hpp"
#include "SceneFixture.hpp"

using client::events::DestroyReason;
using client::events::DisconnectRequested;
using client::events::EntityDestroyed;
using client::events::EntityKind;
using client::events::EntitySpawned;
using client::events::GamePhase;
using client::events::GameStateChanged;
using client::events::InGameMenuClosed;
using client::events::InGameMenuOpened;
using client::events::ScoreChanged;
using client::events::WelcomeReceived;
using client::scenes::GameOverlay;
using client::scenes::GameOverlays;
using client::scenes::GameOverOverlay;
using client::scenes::InGameMenuOverlay;
using client::scenes::MainMenuScene;
using client::scenes::RespawnOverlay;
using client::scenes::SceneRoute;
using client::scenes::SceneRouter;
using engine::ecs::UIKey;
using engine::ecs::components::UIText;

namespace {

constexpr std::uint8_t US = 1;
constexpr std::uint16_t OUR_SHIP = 40;
constexpr float SECOND = 1.0f;

/** @brief Stands for the game scene: runs the input, counts its updates. */
class FakeGame : public engine::ecs::UIScene {
public:
    FakeGame(engine::ecs::UISceneContext context, GameOverlays &overlays,
             int &updates)
        : UIScene(std::move(context)),
          _overlays(overlays),
          _updates(updates)
    {}

    void update(float dt) override
    {
        _overlays.update(dt);
        ++_updates;
    }

protected:
    void activate() override {}

private:
    GameOverlays &_overlays;
    int &_updates;
};

struct GameFixture : SceneFixture {
    GameFixture()
    {
        client::scenes::addOverlayRoutes(router, context(), overlays);
        router.setRoute(SceneRoute::MAIN_MENU, [this] {
            return std::make_unique<MainMenuScene>(context());
        });
        scenes.pushScene(
            std::make_unique<FakeGame>(context(), overlays, updates));
        bus.subscribe<InGameMenuOpened>(
            [this](const InGameMenuOpened &) { ++opened; });
        bus.subscribe<InGameMenuClosed>(
            [this](const InGameMenuClosed &) { ++closed; });
        bus.subscribe<DisconnectRequested>(
            [this](const DisconnectRequested &) { ++disconnects; });
        bus.publish(WelcomeReceived{US, 4});
        bus.publish(EntitySpawned{OUR_SHIP, EntityKind::PLAYER_SHIP, US});
    }

    void step(float dt = 0.0f)
    {
        layout.update(registry);
        scenes.handleInput();
        scenes.update(dt);
        router.apply();
        layout.update(registry);
    }

    void press(UIKey pressed)
    {
        interaction.onKeyPressed(pressed);
        step();
    }

    void killUs()
    {
        bus.publish(EntityDestroyed{OUR_SHIP, DestroyReason::KILLED, 2});
        step();
    }

    void respawnUs()
    {
        bus.publish(EntitySpawned{OUR_SHIP, EntityKind::PLAYER_SHIP, US});
        step();
    }

    template <typename Scene>
    Scene *top()
    {
        return dynamic_cast<Scene *>(&scenes.currentScene());
    }

    std::string text(engine::ecs::Entity entity)
    {
        return registry.getComponents<UIText>().get(entity)->text;
    }

    GameOverlays overlays{bus};
    SceneRouter router{scenes, bus};
    int updates = 0;
    int opened = 0;
    int closed = 0;
    int disconnects = 0;
};

} // namespace

TEST_CASE("Escape opens and closes the in-game menu",
          "[client][scenes][overlay]")
{
    GameFixture game;

    game.press(UIKey::ESCAPE);
    REQUIRE(game.top<InGameMenuOverlay>() != nullptr);
    REQUIRE(game.scenes.currentScene().isOverlay());
    REQUIRE(game.opened == 1);

    game.press(UIKey::ESCAPE);
    REQUIRE(game.top<FakeGame>() != nullptr);
    REQUIRE(game.closed == 1);
    REQUIRE(game.scenes.size() == 1);
}

TEST_CASE("the world keeps updating behind the menu",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.press(UIKey::ESCAPE);
    const int before = game.updates;

    game.step();
    game.step();

    REQUIRE(game.updates == before + 2);
}

TEST_CASE("Resume closes the menu", "[client][scenes][overlay]")
{
    GameFixture game;
    game.press(UIKey::ESCAPE);

    game.click(game.top<InGameMenuOverlay>()->button("Resume"));
    game.router.apply();

    REQUIRE(game.top<FakeGame>() != nullptr);
    REQUIRE(game.closed == 1);
}

TEST_CASE("Disconnect goes through events and back to the menu",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.press(UIKey::ESCAPE);

    game.click(game.top<InGameMenuOverlay>()->button("Disconnect"));
    game.router.apply();

    REQUIRE(game.disconnects == 1);
    REQUIRE(game.closed == 1);
    REQUIRE(game.top<MainMenuScene>() != nullptr);
    REQUIRE(game.scenes.size() == 1);
}

TEST_CASE("Settings and Controls wait for their screens",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.press(UIKey::ESCAPE);

    game.click(game.top<InGameMenuOverlay>()->button("Settings"));
    game.router.apply();
    game.click(game.top<InGameMenuOverlay>()->button("Controls"));
    game.router.apply();

    REQUIRE(game.top<InGameMenuOverlay>() != nullptr);
    REQUIRE(game.scenes.size() == 2);
}

TEST_CASE("the countdown opens on our death and closes on our respawn",
          "[client][scenes][overlay]")
{
    GameFixture game;

    game.killUs();
    REQUIRE(game.top<RespawnOverlay>() != nullptr);
    REQUIRE(game.overlays.respawnSecondsLeft() == 2);
    const auto label = *game.top<RespawnOverlay>()->label();
    REQUIRE(game.text(label) == "Respawning in 2");

    game.step(1.5f * SECOND);
    REQUIRE(game.text(label) == "Respawning in 1");
    game.step(5.0f * SECOND);
    REQUIRE(game.text(label) == "Respawning in 0");
    REQUIRE(game.top<RespawnOverlay>() != nullptr);

    game.respawnUs();
    REQUIRE(game.top<FakeGame>() != nullptr);
}

TEST_CASE("only our own ship killed opens the countdown",
          "[client][scenes][overlay]")
{
    GameFixture game;

    game.bus.publish(EntityDestroyed{99, DestroyReason::KILLED, 2});
    game.step();
    REQUIRE(game.overlays.active() == GameOverlay::NONE);

    game.bus.publish(EntityDestroyed{OUR_SHIP, DestroyReason::GONE, 2});
    game.step();
    REQUIRE(game.overlays.active() == GameOverlay::NONE);
}

TEST_CASE("someone else's spawn does not close the countdown",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.killUs();

    game.bus.publish(EntitySpawned{41, EntityKind::PLAYER_SHIP, 2});
    game.bus.publish(EntitySpawned{42, EntityKind::BYDOS, US});
    game.step();

    REQUIRE(game.top<RespawnOverlay>() != nullptr);
}

TEST_CASE("the menu covers the countdown, which keeps running",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.killUs();

    game.press(UIKey::ESCAPE);
    REQUIRE(game.top<InGameMenuOverlay>() != nullptr);
    REQUIRE(game.scenes.size() == 2);

    game.step(1.5f * SECOND);
    game.press(UIKey::ESCAPE);
    REQUIRE(game.top<RespawnOverlay>() != nullptr);
    REQUIRE(game.overlays.respawnSecondsLeft() == 1);
}

TEST_CASE("a respawn under the menu leaves the menu open",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.killUs();
    game.press(UIKey::ESCAPE);

    game.respawnUs();
    REQUIRE(game.top<InGameMenuOverlay>() != nullptr);

    game.press(UIKey::ESCAPE);
    REQUIRE(game.top<FakeGame>() != nullptr);
    REQUIRE(game.scenes.size() == 1);
}

TEST_CASE("game over only shows on GAME_OVER, with our final score",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.bus.publish(ScoreChanged{US, 79500});
    game.bus.publish(ScoreChanged{2, 111});

    game.bus.publish(GameStateChanged{GamePhase::WAITING, 1});
    game.bus.publish(GameStateChanged{GamePhase::PLAYING, 1});
    game.step();
    REQUIRE(game.overlays.active() == GameOverlay::NONE);

    game.bus.publish(GameStateChanged{GamePhase::GAME_OVER, 1});
    game.step();
    REQUIRE(game.top<GameOverOverlay>() != nullptr);
    REQUIRE(game.text(*game.top<GameOverOverlay>()->scoreLabel()) ==
            "Final score 0079500");
}

TEST_CASE("game over wins over the menu and the countdown",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.killUs();
    game.press(UIKey::ESCAPE);

    game.bus.publish(GameStateChanged{GamePhase::GAME_OVER, 1});
    game.step();
    REQUIRE(game.top<GameOverOverlay>() != nullptr);
    REQUIRE(game.closed == 1);
    REQUIRE(game.scenes.size() == 2);

    game.press(UIKey::ESCAPE);
    REQUIRE(game.top<GameOverOverlay>() != nullptr);
}

TEST_CASE("Back to menu leaves the game", "[client][scenes][overlay]")
{
    GameFixture game;
    game.bus.publish(GameStateChanged{GamePhase::GAME_OVER, 1});
    game.step();

    game.click(*game.top<GameOverOverlay>()->backButton());
    game.router.apply();

    REQUIRE(game.disconnects == 1);
    REQUIRE(game.top<MainMenuScene>() != nullptr);
    REQUIRE(game.scenes.size() == 1);
}

TEST_CASE("popped overlays remove their widgets and listeners",
          "[client][scenes][overlay]")
{
    GameFixture game;

    game.press(UIKey::ESCAPE);
    REQUIRE(game.registry.aliveCount() > 0);
    game.press(UIKey::ESCAPE);
    REQUIRE(game.registry.aliveCount() == 0);
    game.killUs();
    game.respawnUs();
    REQUIRE(game.registry.aliveCount() == 0);

    game.bus.publish(engine::ecs::components::UICancelEvent{});
    game.bus.publish(client::events::ResumeRequested{});
    game.router.apply();
    REQUIRE(game.closed == 2);
}

TEST_CASE("game over closes everything above the game, settings included",
          "[client][scenes][overlay]")
{
    GameFixture game;
    game.press(UIKey::ESCAPE);
    game.bus.publish(
        client::scenes::OverlayRequested{SceneRoute::IN_GAME_MENU});
    game.step();
    REQUIRE(game.scenes.size() == 3);

    game.bus.publish(GameStateChanged{GamePhase::GAME_OVER, 1});
    game.step();

    REQUIRE(game.top<GameOverOverlay>() != nullptr);
    REQUIRE(game.scenes.size() == 2);
}
