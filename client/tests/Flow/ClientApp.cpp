/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientApp.cpp
*/

#include "Client/Flow/ClientApp.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>

#include "Client/Scenes/ConnectScene.hpp"
#include "Client/Scenes/MainMenuScene.hpp"
#include "Client/Scenes/NoticeOverlay.hpp"
#include "Client/Scenes/SettingsScene.hpp"
#include "Engine/Ecs/Components/UIComponents.hpp"

using client::events::ConnectionLost;
using client::flow::AppServices;
using client::flow::ClientApp;
using client::scenes::ConnectScene;
using client::scenes::MainMenuScene;
using client::scenes::NoticeOverlay;
using client::scenes::SettingsScene;
using engine::ecs::UIKey;

namespace {

class SilentDevice : public client::flow::IGameDevice {
public:
    [[nodiscard]] bool isKeyDown(client::settings::ScanCode) const override
    {
        return false;
    }

    void playShot() override {}

    void playExplosion() override {}

    [[nodiscard]] engine::core::Vec2
    toScreen(engine::core::Vec2 world) const override
    {
        return world;
    }

    [[nodiscard]] engine::core::Vec2 size() const override
    {
        return {1280.0f, 720.0f};
    }
};

struct AppFixture {
    AppFixture()
        : store(std::filesystem::temp_directory_path() /
                ("rtype-flow-" + std::to_string(counter++) + ".cfg")),
          app(AppServices{device,
                          registry,
                          bus,
                          interaction,
                          layout,
                          theme,
                          store,
                          "font.ttf",
                          {"127.0.0.1", "9", "Bob"}})
    {}

    ~AppFixture()
    {
        app.shutdown();
        std::error_code error;
        std::filesystem::remove(path, error);
    }

    template <typename Scene>
    [[nodiscard]] const Scene *scene() const
    {
        return dynamic_cast<const Scene *>(&app.currentScene());
    }

    void click(engine::ecs::Entity entity)
    {
        layout.update(registry);
        const auto &bounds = registry.getComponents<engine::ecs::UITransform>()
                                 .get(entity)
                                 ->actualBounds;
        const float x = bounds.x + bounds.width / 2.0f;
        const float y = bounds.y + bounds.height / 2.0f;

        interaction.onMouseDown(x, y);
        interaction.onMouseUp(x, y);
        app.frame(1.0f / 60.0f);
    }

    void key(UIKey pressed)
    {
        interaction.onKeyPressed(pressed);
        app.frame(1.0f / 60.0f);
    }

    static inline int counter = 0;
    std::filesystem::path path;
    SilentDevice device;
    engine::ecs::Registry registry = [] {
        engine::ecs::Registry created;

        engine::ecs::registerUIComponents(created);
        return created;
    }();
    engine::core::EventBus bus;
    engine::ecs::UIInteractionSystem interaction{bus};
    engine::ecs::UILayoutSystem layout{1280.0f, 720.0f};
    engine::ecs::UITheme theme;
    client::settings::SettingsStore store;
    ClientApp app;
};

} // namespace

TEST_CASE("the client starts on the main menu", "[client][flow][app]")
{
    AppFixture fixture;

    REQUIRE(fixture.scene<MainMenuScene>() != nullptr);
    REQUIRE_FALSE(fixture.app.gameFrame().has_value());
    REQUIRE_FALSE(fixture.app.quitRequested());
}

TEST_CASE("Play opens the form prefilled with the command line",
          "[client][flow][app]")
{
    AppFixture fixture;

    fixture.click(fixture.scene<MainMenuScene>()->button("Play"));
    const auto *form = fixture.scene<ConnectScene>();

    REQUIRE(form != nullptr);
    REQUIRE(form->form().host == "127.0.0.1");
    REQUIRE(form->form().port == "9");
    REQUIRE(form->form().nickname == "Bob");
    fixture.key(UIKey::ESCAPE);
    REQUIRE(fixture.scene<MainMenuScene>() != nullptr);
}

TEST_CASE("connecting starts a session that Back gives up",
          "[client][flow][app]")
{
    AppFixture fixture;

    fixture.click(fixture.scene<MainMenuScene>()->button("Play"));
    fixture.click(fixture.scene<ConnectScene>()->connectButton());
    REQUIRE(fixture.app.isConnecting());
    REQUIRE(fixture.scene<ConnectScene>()->isConnecting());
    fixture.click(fixture.scene<ConnectScene>()->backButton());
    REQUIRE_FALSE(fixture.app.isConnecting());
    REQUIRE(fixture.scene<MainMenuScene>() != nullptr);
}

TEST_CASE("Quit asks the main loop to stop", "[client][flow][app]")
{
    AppFixture fixture;

    fixture.click(fixture.scene<MainMenuScene>()->button("Quit"));
    REQUIRE(fixture.app.quitRequested());
}

TEST_CASE("Settings opens above the menu and Escape comes back",
          "[client][flow][app]")
{
    AppFixture fixture;

    fixture.click(fixture.scene<MainMenuScene>()->button("Settings"));
    REQUIRE(fixture.scene<SettingsScene>() != nullptr);
    fixture.key(UIKey::ESCAPE);
    REQUIRE(fixture.scene<MainMenuScene>() != nullptr);
}

TEST_CASE("a lost connection returns to the menu with a notice",
          "[client][flow][app]")
{
    AppFixture fixture;

    fixture.click(fixture.scene<MainMenuScene>()->button("Play"));
    fixture.bus.publish(ConnectionLost{});
    fixture.app.frame(1.0f / 60.0f);
    const auto *notice = fixture.scene<NoticeOverlay>();

    REQUIRE(notice != nullptr);
    REQUIRE(notice->message() == fixture.app.notice());
    REQUIRE_FALSE(fixture.app.notice().empty());
    fixture.click(notice->okButton());
    REQUIRE(fixture.scene<MainMenuScene>() != nullptr);
}
