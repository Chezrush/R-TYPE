/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConnectScene.cpp
*/

#include "Client/Scenes/ConnectScene.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Client/Events/GameEvents.hpp"
#include "SceneFixture.hpp"

using client::events::ConnectCancelled;
using client::events::ConnectionFailed;
using client::events::ConnectionFailure;
using client::events::ConnectionStarted;
using client::events::ConnectRequested;
using client::events::WelcomeReceived;
using client::scenes::ConnectForm;
using client::scenes::ConnectScene;
using client::scenes::SceneRoute;
using engine::ecs::UIInteractionSystem;
using engine::ecs::UIKey;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonState;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextInput;

namespace {

struct ConnectFixture : SceneFixture {
    ConnectFixture()
    {
        bus.subscribe<ConnectRequested>([this](const ConnectRequested &event) {
            requests.push_back(event);
        });
        bus.subscribe<ConnectCancelled>(
            [this](const ConnectCancelled &) { ++cancels; });
        auto owned = std::make_unique<ConnectScene>(
            context(), ConnectForm{"127.0.0.1", "4242", "Bob"});
        scene = owned.get();
        scenes.pushScene(std::move(owned));
        frame();
    }

    [[nodiscard]] const UITextInput &field(engine::ecs::Entity entity) const
    {
        return *registry.getComponents<UITextInput>().get(entity);
    }

    [[nodiscard]] const UIButton &button(engine::ecs::Entity entity) const
    {
        return *registry.getComponents<UIButton>().get(entity);
    }

    [[nodiscard]] std::string status() const
    {
        return registry.getComponents<UIText>().get(scene->statusLabel())->text;
    }

    ConnectScene *scene = nullptr;
    std::vector<ConnectRequested> requests;
    int cancels = 0;
};

} // namespace

TEST_CASE("the form starts with the default address and port",
          "[client][scenes][connect]")
{
    SceneFixture fixture;
    fixture.scenes.pushScene(std::make_unique<ConnectScene>(fixture.context()));
    const auto &scene =
        dynamic_cast<const ConnectScene &>(fixture.scenes.currentScene());

    REQUIRE(scene.form().host == "127.0.0.1");
    REQUIRE(scene.form().port == "4242");
    REQUIRE(scene.form().nickname.empty());
    REQUIRE(UIInteractionSystem::focusedEntity(fixture.registry) ==
            scene.hostField());
}

TEST_CASE("Connect publishes the content of the form",
          "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.click(fixture.scene->connectButton());

    REQUIRE(fixture.requests.size() == 1);
    REQUIRE(fixture.requests[0].host == "127.0.0.1");
    REQUIRE(fixture.requests[0].port == 4242);
    REQUIRE(fixture.requests[0].nickname == "Bob");
    REQUIRE(fixture.scene->isConnecting());
    REQUIRE(fixture.status() == "Connecting to 127.0.0.1:4242...");
}

TEST_CASE("the form is usable with the keyboard alone",
          "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.key(UIKey::END);
    fixture.key(UIKey::BACKSPACE);
    fixture.type(U"2");
    fixture.key(UIKey::TAB);
    fixture.key(UIKey::TAB);
    REQUIRE(UIInteractionSystem::focusedEntity(fixture.registry) ==
            fixture.scene->nicknameField());
    fixture.type(U"by");
    fixture.key(UIKey::ENTER);

    REQUIRE(fixture.requests.size() == 1);
    REQUIRE(fixture.requests[0].host == "127.0.0.2");
    REQUIRE(fixture.requests[0].nickname == "Bobby");
}

TEST_CASE("an invalid form stays editable and sends nothing",
          "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.key(UIKey::TAB);
    for (int i = 0; i < 4; ++i)
        fixture.key(UIKey::BACKSPACE);
    fixture.key(UIKey::ENTER);

    REQUIRE(fixture.requests.empty());
    REQUIRE_FALSE(fixture.scene->isConnecting());
    REQUIRE(fixture.status().find("port") != std::string::npos);
    REQUIRE(UIInteractionSystem::focusedEntity(fixture.registry) ==
            fixture.scene->portField());
}

TEST_CASE("the form is locked while connecting", "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.click(fixture.scene->connectButton());

    REQUIRE_FALSE(fixture.field(fixture.scene->hostField()).enabled);
    REQUIRE(fixture.button(fixture.scene->connectButton()).state ==
            UIButtonState::DISABLED);
    REQUIRE(UIInteractionSystem::focusedEntity(fixture.registry) ==
            fixture.scene->backButton());
    fixture.click(fixture.scene->connectButton());
    REQUIRE(fixture.requests.size() == 1);
}

TEST_CASE("the network layer can report the progress",
          "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.click(fixture.scene->connectButton());
    fixture.bus.publish(ConnectionStarted{"10.0.0.1", 5000});

    REQUIRE(fixture.status() == "Connecting to 10.0.0.1:5000...");
}

TEST_CASE("every failure gives a message and the form back",
          "[client][scenes][connect]")
{
    for (const ConnectionFailure reason :
         {ConnectionFailure::SERVER_FULL, ConnectionFailure::INVALID_NAME,
          ConnectionFailure::VERSION, ConnectionFailure::RATE_LIMITED,
          ConnectionFailure::SHUTTING_DOWN, ConnectionFailure::TIMEOUT,
          ConnectionFailure::INVALID_ADDRESS}) {
        ConnectFixture fixture;

        fixture.click(fixture.scene->connectButton());
        fixture.bus.publish(ConnectionFailed{reason});

        REQUIRE(fixture.status() == client::scenes::failureMessage(reason));
        REQUIRE_FALSE(fixture.scene->isConnecting());
        REQUIRE(fixture.field(fixture.scene->hostField()).enabled);
        REQUIRE(fixture.button(fixture.scene->connectButton()).state ==
                UIButtonState::NORMAL);
        fixture.key(UIKey::ENTER);
        REQUIRE(fixture.requests.size() == 2);
    }
}

TEST_CASE("welcome goes to the game", "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.bus.publish(WelcomeReceived{});
    REQUIRE(fixture.navigations == 0);

    fixture.click(fixture.scene->connectButton());
    fixture.bus.publish(WelcomeReceived{});
    REQUIRE(fixture.navigations == 1);
    REQUIRE(fixture.route == SceneRoute::GAME);
}

TEST_CASE("a failure is ignored when nothing is in progress",
          "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.bus.publish(ConnectionFailed{ConnectionFailure::TIMEOUT});

    REQUIRE(fixture.status().empty());
}

TEST_CASE("Back and Escape return to the menu", "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.click(fixture.scene->backButton());
    REQUIRE(fixture.route == SceneRoute::MAIN_MENU);
    REQUIRE(fixture.navigations == 1);
    REQUIRE(fixture.cancels == 0);

    fixture.key(UIKey::ESCAPE);
    REQUIRE(fixture.navigations == 2);
}

TEST_CASE("leaving during a connection cancels it", "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.click(fixture.scene->connectButton());
    fixture.key(UIKey::ENTER);

    REQUIRE(fixture.cancels == 1);
    REQUIRE(fixture.route == SceneRoute::MAIN_MENU);
}

TEST_CASE("the form survives a pause", "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.key(UIKey::TAB);
    fixture.key(UIKey::TAB);
    fixture.type(U"by");
    fixture.scenes.currentScene().onPause();
    fixture.scenes.currentScene().onResume();

    REQUIRE(fixture.field(fixture.scene->nicknameField()).text == "Bobby");
}

TEST_CASE("listeners and widgets are gone once the scene is popped",
          "[client][scenes][connect]")
{
    ConnectFixture fixture;

    fixture.click(fixture.scene->connectButton());
    fixture.scenes.popScene();

    REQUIRE(fixture.registry.aliveCount() == 0);
    fixture.bus.publish(WelcomeReceived{});
    fixture.bus.publish(ConnectionFailed{ConnectionFailure::TIMEOUT});
    REQUIRE(fixture.navigations == 0);
}
