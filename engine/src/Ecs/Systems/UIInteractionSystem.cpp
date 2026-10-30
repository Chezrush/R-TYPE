/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIInteractionSystem.cpp
*/

#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"

#include <algorithm>
#include <utility>

#include "Engine/Ecs/IndexedZipper.hpp"
#include "Engine/Ecs/Zipper.hpp"

namespace engine::ecs::systems {

namespace {

using components::UIButton;
using components::UIButtonClickedEvent;
using components::UIButtonState;
using components::UITransform;
using Buttons = SparseArray<UIButton>;

bool isEnabled(const UIButton &button) noexcept
{
    return button.state != UIButtonState::DISABLED;
}

void clearFocus(Buttons &buttons)
{
    for (auto [button] : makeZipper(buttons))
        button.isFocused = false;
}

void dropDisabledFocus(Buttons &buttons)
{
    for (auto [button] : makeZipper(buttons)) {
        if (!isEnabled(button))
            button.isFocused = false;
    }
}

std::vector<UIButton *> enabledButtons(Buttons &buttons)
{
    std::vector<UIButton *> enabled;

    for (auto [button] : makeZipper(buttons)) {
        if (isEnabled(button))
            enabled.push_back(&button);
    }
    return enabled;
}

void moveFocus(Buttons &buttons, bool forward)
{
    const auto enabled = enabledButtons(buttons);
    const auto current =
        std::find_if(enabled.begin(), enabled.end(),
                     [](const UIButton *button) { return button->isFocused; });
    const std::size_t count = enabled.size();

    clearFocus(buttons);
    if (count == 0)
        return;
    std::size_t next = forward ? 0 : count - 1;
    if (current != enabled.end())
        next = (static_cast<std::size_t>(current - enabled.begin()) +
                (forward ? 1 : count - 1)) %
               count;
    enabled[next]->isFocused = true;
}

UIButtonState hoverState(bool inside, bool pressed) noexcept
{
    if (pressed)
        return inside ? UIButtonState::PRESSED : UIButtonState::NORMAL;
    return inside ? UIButtonState::HOVERED : UIButtonState::NORMAL;
}

} // namespace

UIInteractionSystem::UIInteractionSystem(core::EventBus &eventBus) noexcept
    : _eventBus(eventBus)
{}

void UIInteractionSystem::onMouseMove(float mouseX, float mouseY)
{
    _pending.push_back({Input::Type::MOVE, mouseX, mouseY, UIKey::TAB});
}

void UIInteractionSystem::onMouseDown(float mouseX, float mouseY)
{
    _pending.push_back({Input::Type::DOWN, mouseX, mouseY, UIKey::TAB});
}

void UIInteractionSystem::onMouseUp(float mouseX, float mouseY)
{
    _pending.push_back({Input::Type::UP, mouseX, mouseY, UIKey::TAB});
}

void UIInteractionSystem::onKeyPressed(UIKey key)
{
    _pending.push_back({Input::Type::KEY, 0.0f, 0.0f, key});
}

void UIInteractionSystem::process(Registry &registry)
{
    const std::vector<Input> pending = std::exchange(_pending, {});

    for (const Input &input : pending)
        apply(input, registry);
}

Scheduler::SystemId UIInteractionSystem::addTo(Scheduler &scheduler,
                                               Phase phase)
{
    Access access;

    access.read<UITransform>().write<UIButton>();
    return scheduler.addSystem("ui_interaction", phase, access,
                               [this](Registry &registry, CommandBuffer &,
                                      double) { process(registry); });
}

void UIInteractionSystem::apply(const Input &input, Registry &registry)
{
    dropDisabledFocus(registry.getComponents<UIButton>());
    switch (input.type) {
        case Input::Type::MOVE:
            pointerMoved(input.x, input.y, registry);
            break;
        case Input::Type::DOWN:
            pointerPressed(input.x, input.y, registry);
            break;
        case Input::Type::UP:
            pointerReleased(input.x, input.y, registry);
            break;
        case Input::Type::KEY:
            keyPressed(input.key, registry);
            break;
    }
}

void UIInteractionSystem::pointerMoved(float x, float y, Registry &registry)
{
    auto &buttons = registry.getComponents<UIButton>();
    const auto &transforms =
        std::as_const(registry).getComponents<UITransform>();

    for (auto [index, transform, button] :
         makeIndexedZipper(transforms, buttons)) {
        if (!isEnabled(button))
            continue;
        const bool pressed = _pressed.has_value() && _pressed->index() == index;
        button.state =
            hoverState(contains(transform.actualBounds, x, y), pressed);
    }
}

void UIInteractionSystem::pointerPressed(float x, float y, Registry &registry)
{
    const auto hit = buttonAt(x, y, registry);

    if (!hit.has_value())
        return;
    auto &buttons = registry.getComponents<UIButton>();
    const Entity entity = registry.entityAt(*hit);
    clearFocus(buttons);
    UIButton &button = *buttons.get(entity);
    button.state = UIButtonState::PRESSED;
    button.isFocused = true;
    _pressed = entity;
}

void UIInteractionSystem::pointerReleased(float x, float y, Registry &registry)
{
    const auto pressed = std::exchange(_pressed, std::nullopt);

    if (!pressed.has_value() || !registry.isAlive(*pressed))
        return;
    UIButton *button = registry.getComponents<UIButton>().get(*pressed);
    if (button == nullptr || !isEnabled(*button))
        return;
    const UITransform *transform =
        std::as_const(registry).getComponents<UITransform>().get(*pressed);
    const bool inside =
        transform != nullptr && contains(transform->actualBounds, x, y);
    button->state = inside ? UIButtonState::HOVERED : UIButtonState::NORMAL;
    if (inside)
        _eventBus.publish(
            UIButtonClickedEvent{pressed->index(), button->action});
}

void UIInteractionSystem::keyPressed(UIKey key, Registry &registry)
{
    switch (key) {
        case UIKey::TAB:
        case UIKey::DOWN:
        case UIKey::RIGHT:
            focusNext(registry);
            break;
        case UIKey::UP:
        case UIKey::LEFT:
            focusPrevious(registry);
            break;
        case UIKey::ENTER:
        case UIKey::SPACE:
            activateFocused(registry);
            break;
    }
}

void UIInteractionSystem::activateFocused(Registry &registry)
{
    const auto focused = focusedEntity(registry);

    if (!focused.has_value())
        return;
    const UIButton &button =
        *std::as_const(registry).getComponents<UIButton>().get(*focused);
    _eventBus.publish(UIButtonClickedEvent{focused->index(), button.action});
}

void UIInteractionSystem::focusNext(Registry &registry)
{
    moveFocus(registry.getComponents<UIButton>(), true);
}

void UIInteractionSystem::focusPrevious(Registry &registry)
{
    moveFocus(registry.getComponents<UIButton>(), false);
}

std::optional<Entity> UIInteractionSystem::pressedEntity() const noexcept
{
    return _pressed;
}

std::optional<Entity>
UIInteractionSystem::focusedEntity(const Registry &registry)
{
    for (auto [index, button] :
         makeIndexedZipper(registry.getComponents<UIButton>())) {
        if (isEnabled(button) && button.isFocused)
            return registry.entityAt(index);
    }
    return std::nullopt;
}

bool UIInteractionSystem::contains(const components::Rect &rect, float x,
                                   float y) noexcept
{
    return x >= rect.x && x <= (rect.x + rect.width) && y >= rect.y &&
           y <= (rect.y + rect.height);
}

std::optional<std::size_t>
UIInteractionSystem::buttonAt(float x, float y, const Registry &registry)
{
    for (auto [index, transform, button] :
         makeIndexedZipper(registry.getComponents<UITransform>(),
                           registry.getComponents<UIButton>())) {
        if (isEnabled(button) && contains(transform.actualBounds, x, y))
            return index;
    }
    return std::nullopt;
}

} // namespace engine::ecs::systems
