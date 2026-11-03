/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** StatsOverlay.cpp
*/

#include "Client/Hud/StatsOverlay.hpp"

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <iostream>
#include <string>

#include "Client/Events/SettingsEvents.hpp"
#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Components/UIGraph.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

using client::events::NetStatsUpdated;
using client::events::RawKeyPressed;
using client::events::StatsLine;
using client::hud::StatsOverlay;
using client::hud::StatsWidget;
using engine::ecs::Registry;
using engine::ecs::UIGraph;
using engine::ecs::components::UIText;
using engine::ecs::components::UITransform;

namespace {

struct StatsFixture {
    StatsFixture()
    {
        engine::ecs::registerUIComponents(registry);
    }

    std::string text(engine::ecs::Entity entity) const
    {
        return registry.getComponents<UIText>().get(entity)->text;
    }

    const UIGraph &graph(StatsWidget which) const
    {
        return *registry.getComponents<UIGraph>().get(overlay.widget(which));
    }

    Registry registry;
    engine::core::EventBus bus;
    StatsOverlay overlay{registry, bus, "font.ttf"};
};

} // namespace

TEST_CASE("F3 shows and hides the stats", "[client][hud][stats]")
{
    StatsFixture fixture;

    REQUIRE_FALSE(fixture.overlay.isVisible());
    REQUIRE(fixture.registry.aliveCount() == 0);

    fixture.bus.publish(RawKeyPressed{client::settings::KEY_F3});
    REQUIRE(fixture.overlay.isVisible());
    REQUIRE(fixture.registry.aliveCount() > 0);

    fixture.bus.publish(RawKeyPressed{client::settings::KEY_F3});
    REQUIRE_FALSE(fixture.overlay.isVisible());
    REQUIRE(fixture.registry.aliveCount() == 0);
}

TEST_CASE("other keys do not toggle the stats", "[client][hud][stats]")
{
    StatsFixture fixture;

    fixture.bus.publish(RawKeyPressed{client::settings::KEY_GRAVE});

    REQUIRE_FALSE(fixture.overlay.isVisible());
}

TEST_CASE("frames are recorded while hidden", "[client][hud][stats]")
{
    StatsFixture fixture;

    for (int i = 0; i < 40; ++i)
        fixture.overlay.update(1.0f / 60.0f);
    fixture.overlay.setVisible(true);

    REQUIRE(fixture.graph(StatsWidget::FRAME_GRAPH).samples.size() == 40);
    REQUIRE(fixture.text(fixture.overlay.widget(StatsWidget::FRAME_TEXT))
                .rfind("FPS 60", 0) == 0);
}

TEST_CASE("network values come from NetStatsUpdated", "[client][hud][stats]")
{
    StatsFixture fixture;
    fixture.overlay.setVisible(true);
    REQUIRE(fixture.text(fixture.overlay.widget(StatsWidget::RTT_TEXT)) ==
            "RTT: no data");

    fixture.bus.publish(NetStatsUpdated{80.0f, 2.5f, 2048.0f, 1024.0f, 7});
    fixture.overlay.update(0.3f);

    REQUIRE(fixture.text(fixture.overlay.widget(StatsWidget::RTT_TEXT)) ==
            "RTT 80 ms");
    REQUIRE(fixture.text(fixture.overlay.widget(StatsWidget::LOSS_TEXT)) ==
            "Loss 2.5 %");
    REQUIRE(fixture.text(fixture.overlay.widget(StatsWidget::TRAFFIC_TEXT)) ==
            "In 2.0 KB/s   out 1.0 KB/s   dropped 7");
    REQUIRE(fixture.graph(StatsWidget::RTT_GRAPH).samples ==
            std::vector<float>{80.0f});
    REQUIRE(fixture.graph(StatsWidget::LOSS_GRAPH).samples ==
            std::vector<float>{2.5f});
}

TEST_CASE("texts refresh 4 times per second, graphs every frame",
          "[client][hud][stats]")
{
    StatsFixture fixture;
    fixture.overlay.setVisible(true);

    fixture.bus.publish(NetStatsUpdated{30.0f, 0.0f, 0.0f, 0.0f, 0});
    fixture.overlay.update(0.1f);

    REQUIRE(fixture.text(fixture.overlay.widget(StatsWidget::RTT_TEXT)) ==
            "RTT: no data");
    REQUIRE(fixture.graph(StatsWidget::RTT_GRAPH).samples.size() == 1);
    fixture.overlay.update(0.2f);
    REQUIRE(fixture.text(fixture.overlay.widget(StatsWidget::RTT_TEXT)) ==
            "RTT 30 ms");
}

TEST_CASE("extra lines are listed under the graphs", "[client][hud][stats]")
{
    StatsFixture fixture;
    fixture.bus.publish(StatsLine{"Entities", "42"});
    fixture.overlay.setVisible(true);

    REQUIRE(fixture.text(fixture.overlay.extraLine(0)) == "Entities: 42");
    REQUIRE(fixture.text(fixture.overlay.extraLine(1)).empty());
}

TEST_CASE("the panel grows with the extra lines", "[client][hud][stats]")
{
    StatsFixture fixture;
    fixture.overlay.setVisible(true);
    const auto height = [&fixture] {
        return fixture.registry.getComponents<UITransform>()
            .get(fixture.overlay.widget(StatsWidget::PANEL))
            ->size.y;
    };
    const float empty = height();

    fixture.bus.publish(StatsLine{"Entities", "42"});
    fixture.overlay.update(0.3f);

    REQUIRE(height() > empty);
    const auto &line = *fixture.registry.getComponents<UITransform>().get(
        fixture.overlay.extraLine(0));
    REQUIRE(line.offset.y + line.size.y <= 8.0f + height());
}

TEST_CASE("the stats stay pinned in the window at any UI scale",
          "[client][hud][stats]")
{
    StatsFixture fixture;
    fixture.overlay.setVisible(true);
    engine::ecs::UILayoutSystem layout(1280.0f, 720.0f);

    for (const float scale : {1.0f, 1.5f}) {
        layout.setScale(scale);
        layout.update(fixture.registry);
        for (auto [transform] : engine::ecs::makeZipper(
                 fixture.registry.getComponents<UITransform>())) {
            REQUIRE(transform.actualBounds.x >= 0.0f);
            REQUIRE(transform.actualBounds.y >= 0.0f);
            REQUIRE(transform.actualBounds.x + transform.actualBounds.width <=
                    1280.0f);
            REQUIRE(transform.actualBounds.y + transform.actualBounds.height <=
                    720.0f);
        }
    }
}

TEST_CASE("the overlay cleans up after itself", "[client][hud][stats]")
{
    Registry registry;
    engine::ecs::registerUIComponents(registry);
    engine::core::EventBus bus;
    {
        StatsOverlay overlay(registry, bus, "font.ttf");
        overlay.setVisible(true);
    }
    REQUIRE(registry.aliveCount() == 0);
    bus.publish(RawKeyPressed{client::settings::KEY_F3});
    bus.publish(NetStatsUpdated{});
}

TEST_CASE("cost of one frame of the stats overlay",
          "[.][benchmark][client][hud][stats]")
{
    StatsFixture fixture;
    engine::ecs::UILayoutSystem layout(1280.0f, 720.0f);
    constexpr int FRAMES = 20000;
    fixture.overlay.setVisible(true);
    for (int i = 0; i < 8; ++i)
        fixture.bus.publish(StatsLine{"Line " + std::to_string(i), "1"});

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < FRAMES; ++i) {
        fixture.bus.publish(NetStatsUpdated{40.0f, 1.0f, 2000.0f, 900.0f, 1});
        fixture.overlay.update(1.0f / 60.0f);
        layout.update(fixture.registry);
    }
    const std::chrono::duration<double, std::milli> elapsed =
        std::chrono::steady_clock::now() - start;

    std::cout << "stats overlay: " << elapsed.count() / FRAMES
              << " ms per frame (update + layout, worst case: shown, network "
                 "event every frame)\n";
    REQUIRE(elapsed.count() / FRAMES < 0.2);
}
