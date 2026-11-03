/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleOverlay.cpp
*/

#include "Client/Scenes/ConsoleOverlay.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Client/Events/ConsoleEvents.hpp"
#include "Client/Events/OverlayEvents.hpp"
#include "Client/Events/SettingsEvents.hpp"
#include "Client/Scenes/GameOverlays.hpp"
#include "Client/Scenes/InGameMenuOverlay.hpp"
#include "SceneFixture.hpp"

using client::console::ConsoleService;
using client::events::ConsoleCommandsListed;
using client::events::ConsoleCommandSubmitted;
using client::events::ConsoleLevel;
using client::events::ConsoleOutput;
using client::events::RawKeyPressed;
using client::events::TextEntryEnded;
using client::events::TextEntryStarted;
using client::scenes::ConsoleOverlay;
using client::scenes::SceneRouter;
using client::settings::KEY_GRAVE;
using engine::ecs::UIKey;
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UITextInput;

namespace {

/** @brief Stands for the game scene: counts its updates and its input. */
class FakeGame : public engine::ecs::UIScene {
public:
    FakeGame(engine::ecs::UISceneContext context, int &updates)
        : UIScene(std::move(context)),
          _updates(updates)
    {}

    void update(float /*dt*/) override
    {
        ++_updates;
    }

protected:
    void activate() override {}

private:
    int &_updates;
};

struct ConsoleFixture : SceneFixture {
    ConsoleFixture()
    {
        client::scenes::addConsoleRoute(router, context(), service);
        scenes.pushScene(std::make_unique<FakeGame>(context(), updates));
        bus.subscribe<ConsoleCommandSubmitted>(
            [this](const ConsoleCommandSubmitted &event) {
                commands.push_back(event.line);
            });
        bus.subscribe<TextEntryStarted>(
            [this](const TextEntryStarted &) { ++entryStarted; });
        bus.subscribe<TextEntryEnded>(
            [this](const TextEntryEnded &) { ++entryEnded; });
    }

    ~ConsoleFixture()
    {
        scenes.clear();
    }

    ConsoleFixture(const ConsoleFixture &) = delete;
    ConsoleFixture &operator=(const ConsoleFixture &) = delete;
    ConsoleFixture(ConsoleFixture &&) = delete;
    ConsoleFixture &operator=(ConsoleFixture &&) = delete;

    /** @brief The key above Tab, as the window code delivers it. */
    void pressGrave()
    {
        bus.publish(RawKeyPressed{KEY_GRAVE});
        interaction.onTextEntered(U'`');
        step();
    }

    void step()
    {
        layout.update(registry);
        scenes.handleInput();
        scenes.update(0.016f);
        router.apply();
    }

    void type(const std::u32string &characters)
    {
        for (const char32_t character : characters)
            interaction.onTextEntered(character);
        step();
    }

    void press(UIKey pressed)
    {
        interaction.onKeyPressed(pressed);
        step();
    }

    ConsoleOverlay *console()
    {
        return dynamic_cast<ConsoleOverlay *>(&scenes.currentScene());
    }

    const std::string &line()
    {
        return registry.getComponents<UITextInput>()
            .get(*console()->inputField())
            ->text;
    }

    const UIScrollList &log()
    {
        return *registry.getComponents<UIScrollList>().get(
            *console()->logList());
    }

    ConsoleService service{bus};
    SceneRouter router{scenes, bus};
    int updates = 0;
    int entryStarted = 0;
    int entryEnded = 0;
    std::vector<std::string> commands;
};

} // namespace

TEST_CASE("the key above Tab opens and closes the console",
          "[client][scenes][console]")
{
    ConsoleFixture fixture;

    fixture.pressGrave();
    REQUIRE(fixture.console() != nullptr);
    REQUIRE(fixture.service.isOpen());
    REQUIRE(fixture.line().empty());

    fixture.pressGrave();
    REQUIRE(fixture.console() == nullptr);
    REQUIRE_FALSE(fixture.service.isOpen());
    REQUIRE(fixture.scenes.size() == 1);
}

TEST_CASE("Escape closes the console and nothing else",
          "[client][scenes][console]")
{
    ConsoleFixture fixture;
    client::scenes::GameOverlays overlays(fixture.bus);
    fixture.pressGrave();

    fixture.press(UIKey::ESCAPE);

    REQUIRE(fixture.scenes.size() == 1);
    REQUIRE(overlays.active() == client::scenes::GameOverlay::NONE);
}

TEST_CASE("the game keeps running and gets no input under the console",
          "[client][scenes][console]")
{
    ConsoleFixture fixture;
    fixture.pressGrave();
    const int before = fixture.updates;

    fixture.type(U"move");

    REQUIRE(fixture.updates == before + 1);
    REQUIRE(fixture.line() == "move");
}

TEST_CASE("typing in the console tells the input code",
          "[client][scenes][console]")
{
    ConsoleFixture fixture;

    fixture.pressGrave();
    REQUIRE(fixture.entryStarted == 1);
    REQUIRE(fixture.entryEnded == 0);

    fixture.pressGrave();
    REQUIRE(fixture.entryEnded == 1);
}

TEST_CASE("Enter sends the command and echoes it", "[client][scenes][console]")
{
    ConsoleFixture fixture;
    fixture.pressGrave();

    fixture.type(U"  spawn 3 ");
    fixture.press(UIKey::ENTER);
    fixture.press(UIKey::ENTER);

    REQUIRE(fixture.commands == std::vector<std::string>{"spawn 3"});
    REQUIRE(fixture.line().empty());
    REQUIRE(fixture.log().rows.back() == "> spawn 3");
}

TEST_CASE("output is shown with its level", "[client][scenes][console]")
{
    ConsoleFixture fixture;
    fixture.pressGrave();

    fixture.bus.publish(ConsoleOutput{"bad", ConsoleLevel::ERROR_LINE});

    REQUIRE(fixture.log().rows.back() == "[error] bad");
}

TEST_CASE("output sent while closed is there on opening",
          "[client][scenes][console]")
{
    ConsoleFixture fixture;

    fixture.bus.publish(ConsoleOutput{"server ready", ConsoleLevel::INFO});
    fixture.pressGrave();

    REQUIRE(fixture.log().rows == std::vector<std::string>{"server ready"});
}

TEST_CASE("the log follows the newest line", "[client][scenes][console]")
{
    ConsoleFixture fixture;
    fixture.pressGrave();

    for (int i = 0; i < 25; ++i)
        fixture.bus.publish(
            ConsoleOutput{std::to_string(i), ConsoleLevel::INFO});

    REQUIRE(fixture.log().rows.size() == 25);
    REQUIRE(fixture.log().scrollOffset == 15);
}

TEST_CASE("Up and Down browse the history", "[client][scenes][console]")
{
    ConsoleFixture fixture;
    fixture.pressGrave();
    fixture.type(U"first");
    fixture.press(UIKey::ENTER);
    fixture.type(U"second");
    fixture.press(UIKey::ENTER);
    fixture.type(U"dra");

    fixture.press(UIKey::UP);
    REQUIRE(fixture.line() == "second");
    fixture.press(UIKey::UP);
    REQUIRE(fixture.line() == "first");
    fixture.press(UIKey::DOWN);
    fixture.press(UIKey::DOWN);
    REQUIRE(fixture.line() == "dra");
    fixture.type(U"ft");
    REQUIRE(fixture.line() == "draft");
}

TEST_CASE("Tab completes from the listed commands", "[client][scenes][console]")
{
    ConsoleFixture fixture;
    fixture.bus.publish(ConsoleCommandsListed{{"show_fps", "show_hitboxes"}});
    fixture.pressGrave();

    fixture.type(U"sh");
    fixture.press(UIKey::TAB);
    REQUIRE(fixture.line() == "show_");
    REQUIRE(fixture.log().rows.back() == "show_fps  show_hitboxes");

    fixture.type(U"f");
    fixture.press(UIKey::TAB);
    REQUIRE(fixture.line() == "show_fps ");
    REQUIRE(fixture.console() != nullptr);
}

TEST_CASE("the console survives game over without leaking",
          "[client][scenes][console]")
{
    ConsoleFixture fixture;
    fixture.pressGrave();

    fixture.bus.publish(engine::core::OverlaysClearRequested{});
    fixture.step();

    REQUIRE(fixture.console() == nullptr);
    REQUIRE_FALSE(fixture.service.isOpen());
    REQUIRE(fixture.entryEnded == 1);
    REQUIRE(fixture.registry.aliveCount() == 0);
    fixture.pressGrave();
    REQUIRE(fixture.console() != nullptr);
}
