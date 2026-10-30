/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIInteractionSystem.cpp
*/

#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"

using engine::core::EventBus;
using engine::ecs::Entity;
using engine::ecs::Phase;
using engine::ecs::Registry;
using engine::ecs::Scheduler;
using engine::ecs::components::Rect;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UIButtonState;
using engine::ecs::components::UITransform;
using engine::ecs::systems::UIInteractionSystem;
using engine::ecs::systems::UIKey;

namespace {

constexpr Rect TOP{0.0f, 0.0f, 100.0f, 40.0f};
constexpr Rect BOTTOM{0.0f, 100.0f, 100.0f, 40.0f};

struct UI {
    EventBus bus;
    Registry registry;
    UIInteractionSystem system{bus};
    std::vector<UIButtonClickedEvent> clicks;

    UI()
    {
        engine::ecs::registerUIComponents(registry);
        bus.subscribe<UIButtonClickedEvent>(
            [this](const UIButtonClickedEvent &event) {
                clicks.push_back(event);
            });
    }

    Entity add(Rect bounds, std::string action,
               UIButtonState state = UIButtonState::NORMAL)
    {
        const Entity entity = registry.spawnEntity();
        UITransform transform;
        UIButton button;

        transform.actualBounds = bounds;
        button.action = std::move(action);
        button.state = state;
        registry.addComponent(entity, transform);
        registry.addComponent(entity, button);
        return entity;
    }

    UIButton &button(Entity entity)
    {
        return *registry.getComponents<UIButton>().get(entity);
    }

    void move(float x, float y)
    {
        system.onMouseMove(x, y);
        system.process(registry);
    }

    void press(float x, float y)
    {
        system.onMouseDown(x, y);
        system.process(registry);
    }

    void release(float x, float y)
    {
        system.onMouseUp(x, y);
        system.process(registry);
    }

    void key(UIKey key)
    {
        system.onKeyPressed(key);
        system.process(registry);
    }

    [[nodiscard]] std::vector<std::string> actions() const
    {
        std::vector<std::string> names;

        names.reserve(clicks.size());
        for (const auto &click : clicks)
            names.push_back(click.action);
        return names;
    }

    [[nodiscard]] int focusedCount() const
    {
        int count = 0;

        for (const auto &slot : registry.getComponents<UIButton>())
            count += slot.has_value() && slot->isFocused ? 1 : 0;
        return count;
    }
};

} // namespace

TEST_CASE("button transitions NORMAL -> HOVERED -> NORMAL on mouse move",
          "[ecs][ui][interaction]")
{
    UI ui;
    const Entity start = ui.add({100.0f, 100.0f, 200.0f, 50.0f}, "start");

    REQUIRE(ui.button(start).state == UIButtonState::NORMAL);
    ui.move(150.0f, 120.0f);
    REQUIRE(ui.button(start).state == UIButtonState::HOVERED);
    ui.move(50.0f, 50.0f);
    REQUIRE(ui.button(start).state == UIButtonState::NORMAL);
}

TEST_CASE("full click flow: NORMAL -> HOVERED -> PRESSED -> CLICKED",
          "[ecs][ui][interaction]")
{
    UI ui;
    const Entity play = ui.add({100.0f, 100.0f, 200.0f, 50.0f}, "play_game");

    ui.move(150.0f, 120.0f);
    REQUIRE(ui.button(play).state == UIButtonState::HOVERED);

    ui.press(150.0f, 120.0f);
    REQUIRE(ui.button(play).state == UIButtonState::PRESSED);
    REQUIRE(ui.system.pressedEntity() == play);

    ui.release(150.0f, 120.0f);
    REQUIRE(ui.button(play).state == UIButtonState::HOVERED);
    REQUIRE_FALSE(ui.system.pressedEntity().has_value());
    REQUIRE(ui.clicks.size() == 1);
    REQUIRE(ui.clicks[0].entityIndex == play.index());
    REQUIRE(ui.clicks[0].action == "play_game");
}

TEST_CASE("releasing mouse outside bounds cancels the click",
          "[ecs][ui][interaction]")
{
    UI ui;
    const Entity cancel = ui.add({100.0f, 100.0f, 200.0f, 50.0f}, "cancel");

    ui.press(150.0f, 120.0f);
    REQUIRE(ui.button(cancel).state == UIButtonState::PRESSED);
    ui.release(50.0f, 50.0f);
    REQUIRE(ui.button(cancel).state == UIButtonState::NORMAL);
    REQUIRE(ui.clicks.empty());
}

TEST_CASE("disabled button ignores mouse hover and clicks",
          "[ecs][ui][interaction]")
{
    UI ui;
    const Entity disabled =
        ui.add({100.0f, 100.0f, 200.0f, 50.0f}, "off", UIButtonState::DISABLED);

    ui.move(150.0f, 120.0f);
    REQUIRE(ui.button(disabled).state == UIButtonState::DISABLED);
    ui.press(150.0f, 120.0f);
    REQUIRE(ui.button(disabled).state == UIButtonState::DISABLED);
    ui.release(150.0f, 120.0f);
    REQUIRE(ui.button(disabled).state == UIButtonState::DISABLED);
    REQUIRE(ui.clicks.empty());
}

TEST_CASE("keyboard navigation cycles focus and activates buttons",
          "[ecs][ui][interaction]")
{
    UI ui;
    const Entity first = ui.add(TOP, "btn1");
    const Entity second = ui.add(BOTTOM, "btn2");
    ui.add({0.0f, 200.0f, 100.0f, 40.0f}, "btn3", UIButtonState::DISABLED);
    ui.button(first).isFocused = true;

    REQUIRE(UIInteractionSystem::focusedEntity(ui.registry) == first);
    ui.key(UIKey::TAB);
    REQUIRE(UIInteractionSystem::focusedEntity(ui.registry) == second);
    ui.key(UIKey::TAB);
    REQUIRE(UIInteractionSystem::focusedEntity(ui.registry) == first);
    ui.key(UIKey::ENTER);
    REQUIRE(ui.actions() == std::vector<std::string>{"btn1"});
    ui.key(UIKey::UP);
    REQUIRE(UIInteractionSystem::focusedEntity(ui.registry) == second);
    ui.key(UIKey::SPACE);
    REQUIRE(ui.actions() == std::vector<std::string>{"btn1", "btn2"});
}

TEST_CASE("clicking a button takes the focus from the others",
          "[ecs][ui][interaction][focus]")
{
    UI ui;
    const Entity first = ui.add(TOP, "first");
    const Entity second = ui.add(BOTTOM, "second");
    ui.key(UIKey::TAB);
    REQUIRE(UIInteractionSystem::focusedEntity(ui.registry) == first);

    ui.press(50.0f, 120.0f);
    ui.release(50.0f, 120.0f);

    REQUIRE(ui.focusedCount() == 1);
    REQUIRE(UIInteractionSystem::focusedEntity(ui.registry) == second);
    ui.key(UIKey::ENTER);
    REQUIRE(ui.actions() == std::vector<std::string>{"second", "second"});
}

TEST_CASE("keyboard navigation repairs several focused buttons",
          "[ecs][ui][interaction][focus]")
{
    UI ui;
    ui.button(ui.add(TOP, "first")).isFocused = true;
    ui.button(ui.add(BOTTOM, "second")).isFocused = true;

    ui.key(UIKey::TAB);

    REQUIRE(ui.focusedCount() == 1);
}

TEST_CASE("a button disabled while pressed never clicks",
          "[ecs][ui][interaction][focus]")
{
    UI ui;
    const Entity first = ui.add(TOP, "first");
    ui.press(50.0f, 20.0f);
    ui.button(first).state = UIButtonState::DISABLED;

    ui.release(50.0f, 20.0f);

    REQUIRE(ui.clicks.empty());
    REQUIRE(ui.button(first).state == UIButtonState::DISABLED);
    REQUIRE_FALSE(ui.system.pressedEntity().has_value());
}

TEST_CASE("a disabled button loses its focus", "[ecs][ui][interaction][focus]")
{
    UI ui;
    const Entity first = ui.add(TOP, "first");
    const Entity second = ui.add(BOTTOM, "second");
    ui.key(UIKey::TAB);
    ui.button(first).state = UIButtonState::DISABLED;

    REQUIRE_FALSE(UIInteractionSystem::focusedEntity(ui.registry).has_value());
    ui.key(UIKey::ENTER);
    REQUIRE(ui.clicks.empty());
    REQUIRE_FALSE(ui.button(first).isFocused);

    ui.key(UIKey::TAB);
    REQUIRE(UIInteractionSystem::focusedEntity(ui.registry) == second);
}

TEST_CASE("a pressed button shows NORMAL while the pointer is outside",
          "[ecs][ui][interaction][focus]")
{
    UI ui;
    const Entity first = ui.add(TOP, "first");
    ui.press(50.0f, 20.0f);

    ui.move(500.0f, 500.0f);
    REQUIRE(ui.button(first).state == UIButtonState::NORMAL);
    ui.move(50.0f, 20.0f);
    REQUIRE(ui.button(first).state == UIButtonState::PRESSED);
}

TEST_CASE("focus wraps backwards from no focus to the last button",
          "[ecs][ui][interaction][focus]")
{
    UI ui;
    ui.add(TOP, "first");
    const Entity second = ui.add(BOTTOM, "second");

    ui.key(UIKey::UP);

    REQUIRE(UIInteractionSystem::focusedEntity(ui.registry) == second);
}

TEST_CASE("input is queued until processed, in arrival order",
          "[ecs][ui][interaction][queue]")
{
    UI ui;
    const Entity first = ui.add(TOP, "first");

    ui.system.onMouseDown(50.0f, 20.0f);
    REQUIRE(ui.button(first).state == UIButtonState::NORMAL);

    ui.system.onMouseUp(50.0f, 20.0f);
    ui.system.process(ui.registry);
    REQUIRE(ui.actions() == std::vector<std::string>{"first"});

    ui.system.process(ui.registry);
    REQUIRE(ui.clicks.size() == 1);
}

TEST_CASE("a button killed while pressed is ignored on release",
          "[ecs][ui][interaction][queue]")
{
    UI ui;
    const Entity first = ui.add(TOP, "first");
    ui.press(50.0f, 20.0f);
    ui.registry.killEntity(first);

    ui.release(50.0f, 20.0f);

    REQUIRE(ui.clicks.empty());
}

TEST_CASE("the interaction runs as a scheduler system",
          "[ecs][ui][interaction][scheduler]")
{
    UI ui;
    Scheduler scheduler;
    ui.add(TOP, "first");

    ui.system.addTo(scheduler);
    ui.system.onMouseDown(50.0f, 20.0f);
    ui.system.onMouseUp(50.0f, 20.0f);
    scheduler.run(ui.registry, 1.0 / 60.0);

    REQUIRE(ui.actions() == std::vector<std::string>{"first"});
    REQUIRE(scheduler.findConflicts().empty());
}

TEST_CASE("the interaction declares a write on UIButton",
          "[ecs][ui][interaction][scheduler]")
{
    UI ui;
    Scheduler scheduler;

    ui.system.addTo(scheduler, Phase::UPDATE);
    scheduler.addSystem<UIButton>(
        "other", Phase::UPDATE,
        [](Entity, UIButton &, engine::ecs::CommandBuffer &, double) {});

    REQUIRE(scheduler.findConflicts().size() == 1);
}
