/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIOverlay.cpp
*/

#include "Engine/Ecs/UIOverlay.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <utility>

#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

using engine::core::EventBus;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::UIInteractionSystem;
using engine::ecs::UILayoutSystem;
using engine::ecs::UIOverlay;
using engine::ecs::UISceneContext;
using engine::ecs::components::Rect;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UITransform;

namespace {

class Popup : public UIOverlay {
public:
    using UIOverlay::UIOverlay;

    std::optional<Entity> backdrop;
    std::optional<Entity> button;

protected:
    void activate() override
    {
        backdrop = addBackdrop(UIColor{0, 0, 0, 128});
        button = addButton("Ok", "ok", {0.0f, 0.0f}, {100.0f, 40.0f});
    }
};

struct OverlayWorld {
    OverlayWorld()
    {
        engine::ecs::registerUIComponents(registry);
    }

    UISceneContext context()
    {
        return {registry, bus, interaction, "font.ttf"};
    }

    Registry registry;
    EventBus bus;
    UIInteractionSystem interaction{bus};
};

} // namespace

TEST_CASE("an overlay keeps the scene below updating by default",
          "[ecs][ui][scene][overlay]")
{
    OverlayWorld world;
    const Popup running(world.context());
    const Popup frozen(world.context(), false);

    REQUIRE(running.isOverlay());
    REQUIRE(running.updatesSceneBelow());
    REQUIRE(frozen.isOverlay());
    REQUIRE_FALSE(frozen.updatesSceneBelow());
}

TEST_CASE("the backdrop covers any window, behind the widgets",
          "[ecs][ui][scene][overlay]")
{
    OverlayWorld world;
    Popup popup(world.context());
    popup.onEnter();
    const UILayoutSystem layout(1920.0f, 1080.0f);

    layout.update(world.registry);

    const auto &transforms = world.registry.getComponents<UITransform>();
    const UITransform &backdrop = *transforms.get(*popup.backdrop);
    REQUIRE(backdrop.actualBounds == Rect{0.0f, 0.0f, 1920.0f, 1080.0f});
    REQUIRE(backdrop.zIndex < transforms.get(*popup.button)->zIndex);
    REQUIRE(backdrop.zIndex >= engine::ecs::UI_OVERLAY_Z_INDEX - 1);
    REQUIRE(
        world.registry.getComponents<UIPanel>().get(*popup.backdrop)->color.a ==
        128);
}
