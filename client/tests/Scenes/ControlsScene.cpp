/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ControlsScene.cpp
*/

#include "Client/Scenes/ControlsScene.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Client/Scenes/HowToPlayScene.hpp"
#include "Client/Scenes/InGameMenuOverlay.hpp"
#include "Client/Scenes/MainMenuScene.hpp"
#include "Client/Scenes/SettingsScene.hpp"
#include "SceneFixture.hpp"
#include "TempFile.hpp"

using client::events::BindingsChanged;
using client::events::RawGamepadButtonPressed;
using client::events::RawKeyPressed;
using client::scenes::ControlsScene;
using client::scenes::HowToPlayScene;
using client::scenes::SceneRoute;
using client::scenes::SceneRouter;
using client::settings::ActionBinding;
using client::settings::BeamChargeMode;
using client::settings::GameAction;
using client::settings::GamepadButton;
using client::settings::KeyBindings;
using client::settings::SettingsStore;
using engine::ecs::UIInteractionSystem;
using engine::ecs::UIKey;
using engine::ecs::components::UIDropdown;
using engine::ecs::components::UIText;

namespace {

constexpr client::settings::ScanCode KEY_Z = 0x1D;

struct ControlsFixture : SceneFixture {
    ControlsFixture()
    {
        bus.subscribe<BindingsChanged>([this](const BindingsChanged &event) {
            changes.push_back(event.bindings);
        });
        auto owned = std::make_unique<ControlsScene>(context(), store);
        scene = owned.get();
        scenes.pushScene(std::move(owned));
        frame();
    }

    ~ControlsFixture()
    {
        scenes.clear();
    }

    ControlsFixture(const ControlsFixture &) = delete;
    ControlsFixture &operator=(const ControlsFixture &) = delete;
    ControlsFixture(ControlsFixture &&) = delete;
    ControlsFixture &operator=(ControlsFixture &&) = delete;

    /** @brief A key press as the window code delivers it: raw, then for UI. */
    void pressKey(client::settings::ScanCode key, UIKey uiKey)
    {
        bus.publish(RawKeyPressed{key});
        interaction.onKeyPressed(uiKey);
        frame();
    }

    const KeyBindings &bindings() const
    {
        return store.settings().bindings;
    }

    std::string text(engine::ecs::Entity entity) const
    {
        return registry.getComponents<UIText>().get(entity)->text;
    }

    TempDirectory directory;
    SettingsStore store{directory.path() / "settings.cfg"};
    ControlsScene *scene = nullptr;
    std::vector<KeyBindings> changes;
};

} // namespace

TEST_CASE("each row shows its action and controls",
          "[client][scenes][controls]")
{
    const ControlsFixture fixture;

    REQUIRE(fixture.text(fixture.scene->actionButton(GameAction::MOVE_UP)) ==
            "Move up:  Up / D-pad up");
    REQUIRE(fixture.text(fixture.scene->actionButton(GameAction::SHOOT)) ==
            "Shoot:  Space / South button");
}

TEST_CASE("a row asks for a key, then stores it", "[client][scenes][controls]")
{
    ControlsFixture fixture;

    fixture.click(fixture.scene->actionButton(GameAction::SHOOT));
    REQUIRE(fixture.scene->isCapturing());
    REQUIRE(fixture.text(fixture.scene->captureLabel()).find("Shoot") !=
            std::string::npos);

    fixture.bus.publish(RawKeyPressed{KEY_Z});

    REQUIRE_FALSE(fixture.scene->isCapturing());
    REQUIRE(fixture.bindings().actions[4].key == KEY_Z);
    REQUIRE(fixture.changes.size() == 1);
    REQUIRE(fixture.changes[0] == fixture.bindings());
    REQUIRE(fixture.text(fixture.scene->actionButton(GameAction::SHOOT)) ==
            "Shoot:  Z / South button");
    REQUIRE(fixture.text(fixture.scene->captureLabel()).empty());
}

TEST_CASE("a row can store a gamepad button", "[client][scenes][controls]")
{
    ControlsFixture fixture;

    fixture.click(fixture.scene->actionButton(GameAction::SHOOT));
    fixture.bus.publish(RawGamepadButtonPressed{GamepadButton::WEST});

    REQUIRE(fixture.bindings().actions[4] ==
            ActionBinding{client::settings::KEY_SPACE, GamepadButton::WEST});
    REQUIRE(fixture.changes.size() == 1);
}

TEST_CASE("the captured key does not act on the UI",
          "[client][scenes][controls]")
{
    ControlsFixture fixture;

    fixture.key(UIKey::ENTER);
    REQUIRE(fixture.scene->isCapturing());
    fixture.pressKey(0x28, UIKey::ENTER);

    REQUIRE_FALSE(fixture.scene->isCapturing());
    REQUIRE(fixture.bindings().actions[0].key == 0x28);

    fixture.key(UIKey::DOWN);
    REQUIRE(UIInteractionSystem::focusedEntity(fixture.registry) ==
            fixture.scene->actionButton(GameAction::MOVE_DOWN));
}

TEST_CASE("Escape cancels without closing the screen",
          "[client][scenes][controls]")
{
    ControlsFixture fixture;
    int closes = 0;
    fixture.bus.subscribe<engine::core::OverlayCloseRequested>(
        [&closes](const auto &) { ++closes; });
    fixture.key(UIKey::ENTER);

    fixture.pressKey(client::settings::KEY_ESCAPE, UIKey::ESCAPE);

    REQUIRE_FALSE(fixture.scene->isCapturing());
    REQUIRE(fixture.changes.empty());
    REQUIRE(closes == 0);
}

TEST_CASE("Open menu can be given Escape back", "[client][scenes][controls]")
{
    ControlsFixture fixture;

    fixture.click(fixture.scene->actionButton(GameAction::OPEN_MENU));
    fixture.bus.publish(RawKeyPressed{KEY_Z});
    fixture.frame();
    fixture.click(fixture.scene->actionButton(GameAction::OPEN_MENU));
    fixture.bus.publish(RawKeyPressed{client::settings::KEY_ESCAPE});

    REQUIRE(fixture.bindings() == KeyBindings{});
}

TEST_CASE("the UI ignores input while waiting for a key",
          "[client][scenes][controls]")
{
    ControlsFixture fixture;

    fixture.click(fixture.scene->actionButton(GameAction::SHOOT));
    fixture.click(fixture.scene->backButton());
    fixture.key(UIKey::ESCAPE);

    REQUIRE(fixture.scene->isCapturing());
    REQUIRE(fixture.navigations == 0);
}

TEST_CASE("a key outside a capture changes nothing",
          "[client][scenes][controls]")
{
    ControlsFixture fixture;

    fixture.bus.publish(RawKeyPressed{KEY_Z});

    REQUIRE(fixture.changes.empty());
    REQUIRE(fixture.bindings() == KeyBindings{});
}

TEST_CASE("a conflict shows a warning until it is solved",
          "[client][scenes][controls]")
{
    ControlsFixture fixture;

    fixture.click(fixture.scene->actionButton(GameAction::SHOOT));
    fixture.bus.publish(RawKeyPressed{client::settings::KEY_UP});
    fixture.frame();
    REQUIRE(
        fixture.text(fixture.scene->warningLabel()).find("Move up and Shoot") !=
        std::string::npos);

    fixture.click(fixture.scene->actionButton(GameAction::SHOOT));
    fixture.bus.publish(RawKeyPressed{KEY_Z});
    REQUIRE(fixture.text(fixture.scene->warningLabel()).empty());
}

TEST_CASE("the options publish BindingsChanged", "[client][scenes][controls]")
{
    ControlsFixture fixture;

    fixture.click(fixture.scene->autoFireToggle());
    REQUIRE(fixture.bindings().autoFire);
    fixture.click(fixture.scene->beamDropdown());
    fixture.key(UIKey::DOWN);
    fixture.key(UIKey::ENTER);

    REQUIRE(fixture.bindings().beamMode == BeamChargeMode::TOGGLE);
    REQUIRE(fixture.changes.size() == 2);
    REQUIRE_FALSE(fixture.registry.getComponents<UIDropdown>()
                      .get(fixture.scene->beamDropdown())
                      ->open);
}

TEST_CASE("reset restores and publishes the defaults",
          "[client][scenes][controls]")
{
    ControlsFixture fixture;
    fixture.click(fixture.scene->actionButton(GameAction::SHOOT));
    fixture.bus.publish(RawKeyPressed{KEY_Z});
    fixture.frame();

    fixture.click(fixture.scene->resetButton());

    REQUIRE(fixture.bindings() == KeyBindings{});
    REQUIRE(fixture.changes.back() == KeyBindings{});
    REQUIRE(fixture.text(fixture.scene->actionButton(GameAction::SHOOT)) ==
            "Shoot:  Space / South button");
}

TEST_CASE("the settings reset keeps the bindings", "[client][scenes][controls]")
{
    SceneFixture fixture;
    const TempDirectory directory;
    SettingsStore store(directory.path() / "settings.cfg");
    client::settings::setKey(store.settings().bindings, GameAction::SHOOT,
                             KEY_Z);
    client::scenes::SettingsScene settings(fixture.context(), store,
                                           fixture.layout);
    settings.onEnter();
    fixture.click(settings.control(client::scenes::SettingsControl::RESET));

    REQUIRE(store.settings().bindings.actions[4].key == KEY_Z);
}

TEST_CASE("bindings persist across restarts", "[client][scenes][controls]")
{
    const TempDirectory directory;
    const auto path = directory.path() / "settings.cfg";
    {
        SceneFixture fixture;
        SettingsStore store(path);
        auto owned = std::make_unique<ControlsScene>(fixture.context(), store);
        auto *scene = owned.get();
        fixture.scenes.pushScene(std::move(owned));
        fixture.click(scene->actionButton(GameAction::SHOOT));
        fixture.bus.publish(RawKeyPressed{KEY_Z});
        fixture.scenes.clear();
    }
    SettingsStore restarted(path);

    restarted.load();

    REQUIRE(restarted.settings().bindings.actions[4].key == KEY_Z);
}

TEST_CASE("how to play shows the player's real controls",
          "[client][scenes][howto]")
{
    KeyBindings bindings;
    client::settings::setKey(bindings, GameAction::SHOOT, KEY_Z);
    bindings.autoFire = true;
    bindings.beamMode = BeamChargeMode::TOGGLE;
    SceneFixture fixture;
    HowToPlayScene scene(fixture.context(), bindings);

    scene.onEnter();

    std::vector<std::string> shown;
    for (auto [text] :
         engine::ecs::makeZipper(fixture.registry.getComponents<UIText>()))
        shown.push_back(text.text);
    const auto has = [&shown](const std::string &wanted) {
        return std::find(shown.begin(), shown.end(), wanted) != shown.end();
    };
    REQUIRE(has("Shoot: Z / South button"));
    REQUIRE(has("Move up: Up / D-pad up"));
    REQUIRE(has("Press shoot to start or stop charging your beam."));
    REQUIRE(has("Auto-fire is on: you keep shooting by yourself."));
}

TEST_CASE("how to play closes with Back and Escape", "[client][scenes][howto]")
{
    const KeyBindings bindings;
    SceneFixture fixture;
    int closes = 0;
    fixture.bus.subscribe<engine::core::OverlayCloseRequested>(
        [&closes](const auto &) { ++closes; });
    auto owned = std::make_unique<HowToPlayScene>(fixture.context(), bindings);
    auto *scene = owned.get();
    fixture.scenes.pushScene(std::move(owned));

    fixture.click(*scene->backButton());
    REQUIRE(closes == 1);
    fixture.key(UIKey::ESCAPE);
    REQUIRE(closes == 2);
}

TEST_CASE("both screens open from both menus and return to them",
          "[client][scenes][controls]")
{
    SceneFixture fixture;
    const TempDirectory directory;
    SettingsStore store(directory.path() / "settings.cfg");
    SceneRouter router(fixture.scenes, fixture.bus);
    client::scenes::addControlsRoute(router, fixture.context(), store);
    client::scenes::addHowToPlayRoute(router, fixture.context(),
                                      store.settings().bindings);
    auto owned =
        std::make_unique<client::scenes::MainMenuScene>(fixture.context());
    auto *mainMenu = owned.get();
    fixture.scenes.pushScene(std::move(owned));

    fixture.click(mainMenu->button("Controls"));
    router.apply();
    REQUIRE(dynamic_cast<ControlsScene *>(&fixture.scenes.currentScene()));
    fixture.key(UIKey::ESCAPE);
    router.apply();
    REQUIRE(&fixture.scenes.currentScene() == mainMenu);
    fixture.click(mainMenu->button("How to play"));
    router.apply();
    REQUIRE(dynamic_cast<HowToPlayScene *>(&fixture.scenes.currentScene()));
    fixture.key(UIKey::ESCAPE);
    router.apply();

    auto inGame =
        std::make_unique<client::scenes::InGameMenuOverlay>(fixture.context());
    auto *menu = inGame.get();
    fixture.scenes.pushScene(std::move(inGame));
    fixture.click(menu->button("How to play"));
    router.apply();
    REQUIRE(dynamic_cast<HowToPlayScene *>(&fixture.scenes.currentScene()));
    fixture.key(UIKey::ESCAPE);
    router.apply();
    fixture.click(menu->button("Controls"));
    router.apply();
    REQUIRE(dynamic_cast<ControlsScene *>(&fixture.scenes.currentScene()));
    fixture.scenes.clear();
}
