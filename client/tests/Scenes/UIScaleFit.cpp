/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIScaleFit.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <memory>

#include "Client/Scenes/ConnectScene.hpp"
#include "Client/Scenes/ControlsScene.hpp"
#include "Client/Scenes/GameOverOverlay.hpp"
#include "Client/Scenes/GameOverlays.hpp"
#include "Client/Scenes/HowToPlayScene.hpp"
#include "Client/Scenes/InGameMenuOverlay.hpp"
#include "Client/Scenes/MainMenuScene.hpp"
#include "Client/Scenes/RespawnOverlay.hpp"
#include "Client/Scenes/SettingsScene.hpp"
#include "SceneFixture.hpp"
#include "TempFile.hpp"

using engine::ecs::components::UITransform;

namespace {

constexpr float WIDTH = 1280.0f;
constexpr float HEIGHT = 720.0f;
constexpr float LARGEST_SCALE = 1.5f;

void requireInsideWindow(SceneFixture &fixture)
{
    fixture.layout.setScale(LARGEST_SCALE);
    fixture.layout.update(fixture.registry);
    REQUIRE(fixture.registry.aliveCount() > 0);
    for (auto [transform] : engine::ecs::makeZipper(
             fixture.registry.getComponents<UITransform>())) {
        if (!transform.followsUIScale)
            continue;
        const auto &bounds = transform.actualBounds;
        REQUIRE(bounds.x >= 0.0f);
        REQUIRE(bounds.y >= 0.0f);
        REQUIRE(bounds.x + bounds.width <= WIDTH);
        REQUIRE(bounds.y + bounds.height <= HEIGHT);
    }
}

template <typename Scene, typename... Args>
void requireSceneFits(SceneFixture &fixture, Args &...args)
{
    auto scene = std::make_unique<Scene>(fixture.context(), args...);

    fixture.scenes.pushScene(std::move(scene));
    requireInsideWindow(fixture);
    fixture.scenes.clear();
}

} // namespace

TEST_CASE("every screen fits a 720p window at the largest UI scale",
          "[client][scenes][scale]")
{
    SECTION("main menu")
    {
        SceneFixture fixture;
        requireSceneFits<client::scenes::MainMenuScene>(fixture);
    }
    SECTION("connection")
    {
        SceneFixture fixture;
        requireSceneFits<client::scenes::ConnectScene>(fixture);
    }
    SECTION("in-game menu")
    {
        SceneFixture fixture;
        requireSceneFits<client::scenes::InGameMenuOverlay>(fixture);
    }
    SECTION("respawn")
    {
        SceneFixture fixture;
        const client::scenes::GameOverlays overlays(fixture.bus);
        requireSceneFits<client::scenes::RespawnOverlay>(fixture, overlays);
    }
    SECTION("game over")
    {
        SceneFixture fixture;
        std::uint32_t score = 0;
        requireSceneFits<client::scenes::GameOverOverlay>(fixture, score);
    }
    SECTION("controls")
    {
        SceneFixture fixture;
        const TempDirectory directory;
        client::settings::SettingsStore store(directory.path() / "s.cfg");
        requireSceneFits<client::scenes::ControlsScene>(fixture, store);
    }
    SECTION("how to play")
    {
        SceneFixture fixture;
        const client::settings::KeyBindings bindings;
        requireSceneFits<client::scenes::HowToPlayScene>(fixture, bindings);
    }
    SECTION("settings")
    {
        SceneFixture fixture;
        const TempDirectory directory;
        client::settings::SettingsStore store(directory.path() / "s.cfg");
        requireSceneFits<client::scenes::SettingsScene>(
            fixture, store, fixture.layout, fixture.theme);
    }
}
