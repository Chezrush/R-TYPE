/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SettingsScene.cpp
*/

#include "Client/Scenes/SettingsScene.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>

#include "Client/Scenes/InGameMenuOverlay.hpp"
#include "Client/Scenes/MainMenuScene.hpp"
#include "SceneFixture.hpp"
#include "TempFile.hpp"

using client::events::MuteChanged;
using client::events::Resolution;
using client::events::VideoSettingsChanged;
using client::events::VolumeBus;
using client::events::VolumeChanged;
using client::scenes::SceneRoute;
using client::scenes::SceneRouter;
using client::scenes::SettingsControl;
using client::scenes::SettingsScene;
using client::settings::ClientSettings;
using client::settings::SettingsStore;
using engine::ecs::UIInteractionSystem;
using engine::ecs::UIKey;
using engine::ecs::components::UIDropdown;
using engine::ecs::components::UISlider;
using engine::ecs::components::UIText;
using engine::ecs::components::UIToggle;

namespace {

struct SettingsFixture : SceneFixture {
    SettingsFixture()
    {
        bus.subscribe<VolumeChanged>(
            [this](const VolumeChanged &event) { volumes.push_back(event); });
        bus.subscribe<MuteChanged>(
            [this](const MuteChanged &event) { mutes.push_back(event.muted); });
        bus.subscribe<VideoSettingsChanged>(
            [this](const VideoSettingsChanged &event) {
                videos.push_back(event);
            });
        auto owned =
            std::make_unique<SettingsScene>(context(), store, layout, theme);
        scene = owned.get();
        scenes.pushScene(std::move(owned));
        frame();
    }

    ~SettingsFixture()
    {
        scenes.clear();
    }

    SettingsFixture(const SettingsFixture &) = delete;
    SettingsFixture &operator=(const SettingsFixture &) = delete;
    SettingsFixture(SettingsFixture &&) = delete;
    SettingsFixture &operator=(SettingsFixture &&) = delete;

    engine::ecs::Entity at(SettingsControl control) const
    {
        return scene->control(control);
    }

    void chooseOption(SettingsControl control, int downPresses)
    {
        click(at(control));
        for (int i = 0; i < downPresses; ++i)
            key(UIKey::DOWN);
        key(UIKey::ENTER);
    }

    TempDirectory directory;
    SettingsStore store{directory.path() / "settings.cfg"};
    SettingsScene *scene = nullptr;
    std::vector<VolumeChanged> volumes;
    std::vector<bool> mutes;
    std::vector<VideoSettingsChanged> videos;
};

template <typename Widget>
const Widget &get(const SettingsFixture &fixture, SettingsControl control)
{
    return *fixture.registry.getComponents<Widget>().get(fixture.at(control));
}

} // namespace

TEST_CASE("the widgets start from the stored settings",
          "[client][scenes][settings]")
{
    SceneFixture base;
    const TempDirectory directory;
    SettingsStore store(directory.path() / "settings.cfg");
    store.settings().masterVolume = 35;
    store.settings().muted = true;
    store.settings().resolution = Resolution{1920, 1080};
    SettingsScene scene(base.context(), store, base.layout, base.theme);

    scene.onEnter();

    const auto &components = base.registry;
    REQUIRE(components.getComponents<UISlider>()
                .get(scene.control(SettingsControl::MASTER_VOLUME))
                ->value == 35.0f);
    REQUIRE(components.getComponents<UIToggle>()
                .get(scene.control(SettingsControl::MUTE))
                ->checked);
    REQUIRE(components.getComponents<UIDropdown>()
                .get(scene.control(SettingsControl::RESOLUTION))
                ->selected == 2);
}

TEST_CASE("sliders publish their bus and value", "[client][scenes][settings]")
{
    SettingsFixture fixture;

    REQUIRE(UIInteractionSystem::focusedEntity(fixture.registry) ==
            fixture.at(SettingsControl::MASTER_VOLUME));
    fixture.key(UIKey::LEFT);
    fixture.key(UIKey::DOWN);
    fixture.key(UIKey::LEFT);
    fixture.key(UIKey::DOWN);
    fixture.key(UIKey::RIGHT);

    REQUIRE(fixture.volumes.size() == 3);
    REQUIRE(fixture.volumes[0].bus == VolumeBus::MASTER);
    REQUIRE(fixture.volumes[0].value == 95);
    REQUIRE(fixture.volumes[1].bus == VolumeBus::MUSIC);
    REQUIRE(fixture.volumes[1].value == 75);
    REQUIRE(fixture.volumes[2].bus == VolumeBus::SFX);
    REQUIRE(fixture.volumes[2].value == 85);
    REQUIRE(fixture.store.settings().masterVolume == 95);
    REQUIRE(fixture.store.settings().musicVolume == 75);
    REQUIRE(fixture.store.settings().sfxVolume == 85);
}

TEST_CASE("the mute toggle publishes MuteChanged", "[client][scenes][settings]")
{
    SettingsFixture fixture;

    fixture.click(fixture.at(SettingsControl::MUTE));
    fixture.click(fixture.at(SettingsControl::MUTE));

    REQUIRE(fixture.mutes == std::vector<bool>{true, false});
    REQUIRE_FALSE(fixture.store.settings().muted);
}

TEST_CASE("video changes are published, not applied",
          "[client][scenes][settings]")
{
    SettingsFixture fixture;

    fixture.click(fixture.at(SettingsControl::FULLSCREEN));
    fixture.click(fixture.at(SettingsControl::VSYNC));
    fixture.chooseOption(SettingsControl::RESOLUTION, 2);

    REQUIRE(fixture.videos.size() == 3);
    const VideoSettingsChanged &last = fixture.videos.back();
    REQUIRE(last.fullscreen);
    REQUIRE_FALSE(last.vsync);
    REQUIRE(last.resolution == Resolution{1920, 1080});
    REQUIRE(fixture.store.settings().resolution == Resolution{1920, 1080});
    REQUIRE(fixture.layout.windowWidth() == 1280.0f);
}

TEST_CASE("the UI scale applies at once and publishes nothing",
          "[client][scenes][settings]")
{
    SettingsFixture fixture;

    fixture.chooseOption(SettingsControl::UI_SCALE, 2);

    REQUIRE(fixture.layout.scale() == 1.5f);
    REQUIRE(fixture.store.settings().uiScalePercent == 150);
    REQUIRE(fixture.videos.empty());
    fixture.chooseOption(SettingsControl::UI_SCALE, 0);
    REQUIRE(fixture.layout.scale() == 1.5f);
}

TEST_CASE("reset restores the defaults everywhere",
          "[client][scenes][settings]")
{
    SettingsFixture fixture;
    fixture.key(UIKey::LEFT);
    fixture.click(fixture.at(SettingsControl::MUTE));
    fixture.chooseOption(SettingsControl::UI_SCALE, 1);
    fixture.volumes.clear();

    fixture.click(fixture.at(SettingsControl::RESET));

    REQUIRE(fixture.store.settings() == ClientSettings{});
    REQUIRE(fixture.layout.scale() == 1.0f);
    REQUIRE(fixture.volumes.size() == 3);
    REQUIRE(fixture.mutes.back() == false);
    REQUIRE(get<UISlider>(fixture, SettingsControl::MASTER_VOLUME).value ==
            100.0f);
    REQUIRE_FALSE(get<UIToggle>(fixture, SettingsControl::MUTE).checked);
    REQUIRE(get<UIDropdown>(fixture, SettingsControl::UI_SCALE).selected == 0);
}

TEST_CASE("the screen is written to the file when it exits",
          "[client][scenes][settings]")
{
    SettingsFixture fixture;
    fixture.key(UIKey::LEFT);
    const auto path = fixture.directory.path() / "settings.cfg";
    REQUIRE_FALSE(std::filesystem::exists(path));

    fixture.scenes.popScene();

    SettingsStore restarted(path);
    restarted.load();
    REQUIRE(restarted.settings().masterVolume == 95);
    REQUIRE(fixture.registry.aliveCount() == 0);
}

TEST_CASE("Back and Escape close the screen", "[client][scenes][settings]")
{
    SettingsFixture fixture;
    int closes = 0;
    fixture.bus.subscribe<engine::core::OverlayCloseRequested>(
        [&closes](const auto &) { ++closes; });

    fixture.click(fixture.at(SettingsControl::BACK));
    REQUIRE(closes == 1);
    fixture.key(UIKey::ESCAPE);
    REQUIRE(closes == 2);
}

TEST_CASE("Escape only closes an open dropdown first",
          "[client][scenes][settings]")
{
    SettingsFixture fixture;
    int closes = 0;
    fixture.bus.subscribe<engine::core::OverlayCloseRequested>(
        [&closes](const auto &) { ++closes; });
    fixture.click(fixture.at(SettingsControl::RESOLUTION));

    fixture.key(UIKey::ESCAPE);

    REQUIRE(closes == 0);
    REQUIRE_FALSE(get<UIDropdown>(fixture, SettingsControl::RESOLUTION).open);
}

TEST_CASE("the screen opens from the main menu and returns to it",
          "[client][scenes][settings]")
{
    SceneFixture fixture;
    const TempDirectory directory;
    SettingsStore store(directory.path() / "settings.cfg");
    SceneRouter router(fixture.scenes, fixture.bus);
    client::scenes::addSettingsRoute(router, fixture.context(), store,
                                     fixture.layout, fixture.theme);
    auto owned =
        std::make_unique<client::scenes::MainMenuScene>(fixture.context());
    auto *menu = owned.get();
    fixture.scenes.pushScene(std::move(owned));

    fixture.click(menu->button("Settings"));
    router.apply();
    REQUIRE(dynamic_cast<SettingsScene *>(&fixture.scenes.currentScene()) !=
            nullptr);

    fixture.key(UIKey::ESCAPE);
    router.apply();
    REQUIRE(&fixture.scenes.currentScene() == menu);
    fixture.frame();
    fixture.click(menu->button("Play"));
    REQUIRE(fixture.route == SceneRoute::CONNECT);
}

TEST_CASE("the screen opens from the in-game menu and returns to it",
          "[client][scenes][settings]")
{
    SceneFixture fixture;
    const TempDirectory directory;
    SettingsStore store(directory.path() / "settings.cfg");
    SceneRouter router(fixture.scenes, fixture.bus);
    client::scenes::addSettingsRoute(router, fixture.context(), store,
                                     fixture.layout, fixture.theme);
    auto owned =
        std::make_unique<client::scenes::InGameMenuOverlay>(fixture.context());
    auto *menu = owned.get();
    fixture.scenes.pushScene(std::move(owned));

    fixture.click(menu->button("Settings"));
    router.apply();
    REQUIRE(fixture.scenes.size() == 2);

    fixture.key(UIKey::ESCAPE);
    router.apply();
    REQUIRE(&fixture.scenes.currentScene() == menu);
    REQUIRE(fixture.scenes.size() == 1);
}

TEST_CASE("the palette applies at once and persists",
          "[client][scenes][settings][accessibility]")
{
    SettingsFixture fixture;
    const auto path = fixture.directory.path() / "settings.cfg";

    fixture.chooseOption(SettingsControl::PALETTE, 2);

    using client::settings::ColorPalette;
    REQUIRE(fixture.store.settings().palette == ColorPalette::DEUTERANOPIA);
    const auto expected =
        client::settings::makeTheme(ColorPalette::DEUTERANOPIA, false, false);
    const engine::ecs::components::UIColor unused{};
    REQUIRE(fixture.theme.resolve(engine::ecs::UIColorRole::PLAYER_1, unused) ==
            expected.resolve(engine::ecs::UIColorRole::PLAYER_1, unused));

    fixture.scenes.popScene();
    SettingsStore restarted(path);
    restarted.load();
    REQUIRE(restarted.settings().palette == ColorPalette::DEUTERANOPIA);
}

TEST_CASE("high contrast and reduced motion apply and persist",
          "[client][scenes][settings][accessibility]")
{
    SettingsFixture fixture;
    const auto path = fixture.directory.path() / "settings.cfg";
    std::vector<bool> motions;
    fixture.bus.subscribe<client::events::ReducedMotionChanged>(
        [&motions](const auto &event) { motions.push_back(event.reduced); });

    fixture.click(fixture.at(SettingsControl::HIGH_CONTRAST));
    fixture.click(fixture.at(SettingsControl::REDUCED_MOTION));

    REQUIRE(fixture.theme.options().textPlates);
    REQUIRE(fixture.theme.options().focusThickness > 2.0f);
    REQUIRE(fixture.theme.options().reducedMotion);
    REQUIRE(motions == std::vector<bool>{true});

    fixture.scenes.popScene();
    SettingsStore restarted(path);
    restarted.load();
    REQUIRE(restarted.settings().highContrast);
    REQUIRE(restarted.settings().reducedMotion);
}

TEST_CASE("reset also restores the accessibility options",
          "[client][scenes][settings][accessibility]")
{
    SettingsFixture fixture;
    std::vector<bool> motions;
    fixture.bus.subscribe<client::events::ReducedMotionChanged>(
        [&motions](const auto &event) { motions.push_back(event.reduced); });
    fixture.click(fixture.at(SettingsControl::HIGH_CONTRAST));
    fixture.click(fixture.at(SettingsControl::REDUCED_MOTION));

    fixture.click(fixture.at(SettingsControl::RESET));

    REQUIRE_FALSE(fixture.theme.options().textPlates);
    REQUIRE_FALSE(fixture.theme.options().reducedMotion);
    REQUIRE(motions.back() == false);
    REQUIRE_FALSE(
        get<UIToggle>(fixture, SettingsControl::HIGH_CONTRAST).checked);
}
