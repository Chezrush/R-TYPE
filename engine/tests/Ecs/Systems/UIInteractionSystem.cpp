/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIInteractionSystem.cpp
*/

#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"

#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/EntityAllocator.hpp"
#include "Engine/Ecs/SparseArray.hpp"

using engine::core::EventBus;
using engine::ecs::EntityAllocator;
using engine::ecs::SparseArray;
using engine::ecs::components::Rect;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UIButtonState;
using engine::ecs::components::UITransform;
using engine::ecs::systems::UIInteractionSystem;
using engine::ecs::systems::UIKey;

TEST_CASE("button transitions NORMAL -> HOVERED -> NORMAL on mouse move",
          "[ecs][ui][interaction]")
{
    EventBus eventBus;
    UIInteractionSystem system(eventBus);
    EntityAllocator allocator;

    SparseArray<UITransform> transforms;
    SparseArray<UIButton> buttons;

    const auto e = allocator.create();
    UITransform t;
    t.actualBounds = Rect{100.0f, 100.0f, 200.0f, 50.0f};
    transforms.insert(e, t);

    UIButton b;
    b.action = "start";
    buttons.insert(e, b);

    REQUIRE(buttons.get(e)->state == UIButtonState::NORMAL);

    // Mouse enters bounds
    system.onMouseMove(150.0f, 120.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::HOVERED);

    // Mouse leaves bounds
    system.onMouseMove(50.0f, 50.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::NORMAL);
}

TEST_CASE("full click flow: NORMAL -> HOVERED -> PRESSED -> CLICKED",
          "[ecs][ui][interaction]")
{
    EventBus eventBus;
    UIInteractionSystem system(eventBus);
    EntityAllocator allocator;

    SparseArray<UITransform> transforms;
    SparseArray<UIButton> buttons;

    const auto e = allocator.create();
    UITransform t;
    t.actualBounds = Rect{100.0f, 100.0f, 200.0f, 50.0f};
    transforms.insert(e, t);

    UIButton b;
    b.action = "play_game";
    buttons.insert(e, b);

    std::vector<UIButtonClickedEvent> receivedEvents;
    eventBus.subscribe<UIButtonClickedEvent>(
        [&](const UIButtonClickedEvent &event) {
            receivedEvents.push_back(event);
        }
    );

    // 1. Mouse enters
    system.onMouseMove(150.0f, 120.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::HOVERED);

    // 2. Mouse down
    system.onMouseDown(150.0f, 120.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::PRESSED);
    REQUIRE(system.pressedEntity() == static_cast<std::size_t>(e));

    // 3. Mouse up inside button: emits event
    system.onMouseUp(150.0f, 120.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::HOVERED);
    REQUIRE_FALSE(system.pressedEntity().has_value());

    REQUIRE(receivedEvents.size() == 1);
    REQUIRE(receivedEvents[0].entityIndex == static_cast<std::size_t>(e));
    REQUIRE(receivedEvents[0].action == "play_game");
}

TEST_CASE("releasing mouse outside bounds cancels the click",
          "[ecs][ui][interaction]")
{
    EventBus eventBus;
    UIInteractionSystem system(eventBus);
    EntityAllocator allocator;

    SparseArray<UITransform> transforms;
    SparseArray<UIButton> buttons;

    const auto e = allocator.create();
    UITransform t;
    t.actualBounds = Rect{100.0f, 100.0f, 200.0f, 50.0f};
    transforms.insert(e, t);

    UIButton b;
    b.action = "cancel";
    buttons.insert(e, b);

    bool eventTriggered = false;
    eventBus.subscribe<UIButtonClickedEvent>(
        [&](const UIButtonClickedEvent &) { eventTriggered = true; }
    );

    system.onMouseDown(150.0f, 120.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::PRESSED);

    // Release mouse outside bounds
    system.onMouseUp(50.0f, 50.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::NORMAL);
    REQUIRE_FALSE(eventTriggered);
}

TEST_CASE("disabled button ignores mouse hover and clicks",
          "[ecs][ui][interaction]")
{
    EventBus eventBus;
    UIInteractionSystem system(eventBus);
    EntityAllocator allocator;

    SparseArray<UITransform> transforms;
    SparseArray<UIButton> buttons;

    const auto e = allocator.create();
    UITransform t;
    t.actualBounds = Rect{100.0f, 100.0f, 200.0f, 50.0f};
    transforms.insert(e, t);

    UIButton b;
    b.state = UIButtonState::DISABLED;
    b.action = "disabled_action";
    buttons.insert(e, b);

    bool eventTriggered = false;
    eventBus.subscribe<UIButtonClickedEvent>(
        [&](const UIButtonClickedEvent &) { eventTriggered = true; }
    );

    system.onMouseMove(150.0f, 120.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::DISABLED);

    system.onMouseDown(150.0f, 120.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::DISABLED);

    system.onMouseUp(150.0f, 120.0f, transforms, buttons);
    REQUIRE(buttons.get(e)->state == UIButtonState::DISABLED);
    REQUIRE_FALSE(eventTriggered);
}

TEST_CASE("keyboard navigation cycles focus and activates buttons",
          "[ecs][ui][interaction]")
{
    EventBus eventBus;
    UIInteractionSystem system(eventBus);
    EntityAllocator allocator;

    SparseArray<UIButton> buttons;

    const auto e1 = allocator.create();
    const auto e2 = allocator.create();
    const auto e3 = allocator.create();

    UIButton b1;
    b1.action = "btn1";
    b1.isFocused = true;

    UIButton b2;
    b2.action = "btn2";

    UIButton b3;
    b3.action = "btn3";
    b3.state = UIButtonState::DISABLED; // skipped

    buttons.insert(e1, b1);
    buttons.insert(e2, b2);
    buttons.insert(e3, b3);

    std::vector<std::string> activations;
    eventBus.subscribe<UIButtonClickedEvent>(
        [&](const UIButtonClickedEvent &event) {
            activations.push_back(event.action);
        }
    );

    // Initial focus on button 1
    REQUIRE(system.focusedEntity(buttons) == static_cast<std::size_t>(e1));

    // Tab moves focus to button 2 (skips disabled button 3)
    system.onKeyPressed(UIKey::TAB, buttons);
    REQUIRE(system.focusedEntity(buttons) == static_cast<std::size_t>(e2));

    // Tab wraps around back to button 1
    system.onKeyPressed(UIKey::TAB, buttons);
    REQUIRE(system.focusedEntity(buttons) == static_cast<std::size_t>(e1));

    // Enter activates button 1
    system.onKeyPressed(UIKey::ENTER, buttons);
    REQUIRE(activations == std::vector<std::string>{"btn1"});

    // Up arrow moves focus back to button 2
    system.onKeyPressed(UIKey::UP, buttons);
    REQUIRE(system.focusedEntity(buttons) == static_cast<std::size_t>(e2));

    // Space activates button 2
    system.onKeyPressed(UIKey::SPACE, buttons);
    REQUIRE(activations == std::vector<std::string>{"btn1", "btn2"});
}

