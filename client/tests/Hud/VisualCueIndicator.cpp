/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** VisualCueIndicator.cpp
*/

#include "Client/Hud/VisualCueIndicator.hpp"

#include <catch2/catch_test_macros.hpp>

#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

using client::events::CueDirection;
using client::events::CueIcon;
using client::events::VisualCueRequested;
using client::hud::VisualCueIndicator;
using engine::ecs::Registry;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIText;
using engine::ecs::components::UITransform;

namespace {

struct CueFixture {
    CueFixture()
    {
        engine::ecs::registerUIComponents(registry);
    }

    const std::string &text(const VisualCueIndicator &cues,
                            CueDirection direction) const
    {
        return registry.getComponents<UIText>()
            .get(cues.label(direction))
            ->text;
    }

    Registry registry;
    engine::core::EventBus bus;
};

} // namespace

TEST_CASE("nothing shows before a cue", "[client][hud][cue]")
{
    CueFixture fixture;
    const VisualCueIndicator cues(fixture.registry, fixture.bus, "font.ttf");

    REQUIRE(fixture.text(cues, CueDirection::LEFT).empty());
    REQUIRE(fixture.registry.getComponents<UIPanel>()
                .get(cues.label(CueDirection::LEFT))
                ->color.a == 0);
}

TEST_CASE("a cue shows its icon on its side", "[client][hud][cue]")
{
    CueFixture fixture;
    const VisualCueIndicator cues(fixture.registry, fixture.bus, "font.ttf");

    fixture.bus.publish(
        VisualCueRequested{CueIcon::EXPLOSION, CueDirection::RIGHT});

    REQUIRE(fixture.text(cues, CueDirection::RIGHT) == "Explosion >>");
    REQUIRE(fixture.text(cues, CueDirection::LEFT).empty());
    REQUIRE(fixture.registry.getComponents<UIPanel>()
                .get(cues.label(CueDirection::RIGHT))
                ->color.a > 0);
}

TEST_CASE("every side and icon has its text", "[client][hud][cue]")
{
    REQUIRE(VisualCueIndicator::cueText(CueIcon::ENEMY, CueDirection::LEFT) ==
            "<< Enemy");
    REQUIRE(VisualCueIndicator::cueText(CueIcon::SHOT, CueDirection::TOP) ==
            "^ Shot");
    REQUIRE(VisualCueIndicator::cueText(CueIcon::ALERT, CueDirection::BOTTOM) ==
            "v Alert");
}

TEST_CASE("a cue goes away after its time", "[client][hud][cue]")
{
    CueFixture fixture;
    VisualCueIndicator cues(fixture.registry, fixture.bus, "font.ttf", 1.0f);

    fixture.bus.publish(VisualCueRequested{CueIcon::SHOT, CueDirection::TOP});
    cues.update(0.6f);
    REQUIRE_FALSE(fixture.text(cues, CueDirection::TOP).empty());
    fixture.bus.publish(VisualCueRequested{CueIcon::ALERT, CueDirection::TOP});
    cues.update(0.6f);
    REQUIRE(fixture.text(cues, CueDirection::TOP) == "^ Alert");
    cues.update(0.6f);

    REQUIRE(fixture.text(cues, CueDirection::TOP).empty());
    REQUIRE(fixture.registry.getComponents<UIPanel>()
                .get(cues.label(CueDirection::TOP))
                ->color.a == 0);
}

TEST_CASE("the four sides stay inside the window", "[client][hud][cue]")
{
    CueFixture fixture;
    const VisualCueIndicator cues(fixture.registry, fixture.bus, "font.ttf");
    engine::ecs::UILayoutSystem layout(1280.0f, 720.0f);

    layout.setScale(1.5f);
    layout.update(fixture.registry);

    for (auto [transform] : engine::ecs::makeZipper(
             fixture.registry.getComponents<UITransform>())) {
        REQUIRE(transform.actualBounds.x >= 0.0f);
        REQUIRE(transform.actualBounds.y >= 0.0f);
        REQUIRE(transform.actualBounds.x + transform.actualBounds.width <=
                1280.0f);
    }
}

TEST_CASE("the indicator cleans up after itself", "[client][hud][cue]")
{
    CueFixture fixture;
    {
        const VisualCueIndicator cues(fixture.registry, fixture.bus,
                                      "font.ttf");
        REQUIRE(fixture.registry.aliveCount() == 4);
    }
    REQUIRE(fixture.registry.aliveCount() == 0);
    fixture.bus.publish(VisualCueRequested{});
}
