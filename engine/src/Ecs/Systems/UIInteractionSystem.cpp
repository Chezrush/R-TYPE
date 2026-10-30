/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIInteractionSystem.cpp
*/

#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"

#include <algorithm>
#include <vector>

namespace engine::ecs::systems {

namespace {

using components::UIButton;
using components::UIButtonClickedEvent;
using components::UIButtonState;
using components::UITransform;

template <typename Component>
Component *componentAt(SparseArray<Component> &array,
                       std::size_t index) noexcept
{
    if (index >= array.size())
        return nullptr;
    auto &slot = *(array.begin() + static_cast<std::ptrdiff_t>(index));
    return slot.has_value() ? &*slot : nullptr;
}

template <typename Component>
const Component *componentAt(const SparseArray<Component> &array,
                             std::size_t index) noexcept
{
    if (index >= array.size())
        return nullptr;
    const auto &slot = *(array.begin() + static_cast<std::ptrdiff_t>(index));
    return slot.has_value() ? &*slot : nullptr;
}

bool isEnabled(const UIButton &button) noexcept
{
    return button.state != UIButtonState::DISABLED;
}

UIButton *enabledButtonAt(SparseArray<UIButton> &buttons,
                          std::size_t index) noexcept
{
    UIButton *button = componentAt(buttons, index);

    return button != nullptr && isEnabled(*button) ? button : nullptr;
}

void clearFocus(SparseArray<UIButton> &buttons) noexcept
{
    for (auto &slot : buttons) {
        if (slot.has_value())
            slot->isFocused = false;
    }
}

void dropDisabledFocus(SparseArray<UIButton> &buttons) noexcept
{
    for (auto &slot : buttons) {
        if (slot.has_value() && !isEnabled(*slot))
            slot->isFocused = false;
    }
}

std::vector<std::size_t>
findEnabledButtons(const SparseArray<UIButton> &buttons)
{
    std::vector<std::size_t> enabled;

    for (std::size_t i = 0; i < buttons.size(); ++i) {
        const UIButton *button = componentAt(buttons, i);
        if (button != nullptr && isEnabled(*button))
            enabled.push_back(i);
    }
    return enabled;
}

std::optional<std::size_t>
focusedPosition(const SparseArray<UIButton> &buttons,
                const std::vector<std::size_t> &enabled) noexcept
{
    for (std::size_t i = 0; i < enabled.size(); ++i) {
        const UIButton *button = componentAt(buttons, enabled[i]);
        if (button != nullptr && button->isFocused)
            return i;
    }
    return std::nullopt;
}

void moveFocus(SparseArray<UIButton> &buttons, bool forward)
{
    const auto enabled = findEnabledButtons(buttons);
    const std::size_t count = enabled.size();
    const auto current = focusedPosition(buttons, enabled);
    std::size_t next = forward ? 0 : count - 1;

    clearFocus(buttons);
    if (count == 0)
        return;
    if (current.has_value())
        next = (*current + (forward ? 1 : count - 1)) % count;
    componentAt(buttons, enabled[next])->isFocused = true;
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

bool UIInteractionSystem::contains(const components::Rect &rect, float x,
                                   float y) noexcept
{
    return x >= rect.x && x <= (rect.x + rect.width) && y >= rect.y &&
           y <= (rect.y + rect.height);
}

std::optional<std::size_t>
UIInteractionSystem::buttonAt(float x, float y,
                              const SparseArray<UITransform> &transforms,
                              const SparseArray<UIButton> &buttons)
{
    const std::size_t count = std::min(transforms.size(), buttons.size());

    for (std::size_t i = 0; i < count; ++i) {
        const UITransform *transform = componentAt(transforms, i);
        const UIButton *button = componentAt(buttons, i);
        if (transform != nullptr && button != nullptr && isEnabled(*button) &&
            contains(transform->actualBounds, x, y))
            return i;
    }
    return std::nullopt;
}

void UIInteractionSystem::onMouseMove(
    float mouseX, float mouseY, const SparseArray<UITransform> &transforms,
    SparseArray<UIButton> &buttons)
{
    const std::size_t count = std::min(transforms.size(), buttons.size());

    dropDisabledFocus(buttons);
    for (std::size_t i = 0; i < count; ++i) {
        const UITransform *transform = componentAt(transforms, i);
        UIButton *button = enabledButtonAt(buttons, i);
        if (transform == nullptr || button == nullptr)
            continue;
        button->state =
            hoverState(contains(transform->actualBounds, mouseX, mouseY),
                       _pressedEntity == i);
    }
}

void UIInteractionSystem::onMouseDown(
    float mouseX, float mouseY, const SparseArray<UITransform> &transforms,
    SparseArray<UIButton> &buttons)
{
    const auto hit = buttonAt(mouseX, mouseY, transforms, buttons);

    dropDisabledFocus(buttons);
    if (!hit.has_value())
        return;
    clearFocus(buttons);
    UIButton &button = *componentAt(buttons, *hit);
    button.state = UIButtonState::PRESSED;
    button.isFocused = true;
    _pressedEntity = hit;
}

void UIInteractionSystem::onMouseUp(float mouseX, float mouseY,
                                    const SparseArray<UITransform> &transforms,
                                    SparseArray<UIButton> &buttons)
{
    const auto pressed = _pressedEntity;

    _pressedEntity.reset();
    dropDisabledFocus(buttons);
    if (!pressed.has_value())
        return;
    UIButton *button = enabledButtonAt(buttons, *pressed);
    if (button == nullptr)
        return;
    const UITransform *transform = componentAt(transforms, *pressed);
    const bool inside = transform != nullptr &&
                        contains(transform->actualBounds, mouseX, mouseY);
    button->state = inside ? UIButtonState::HOVERED : UIButtonState::NORMAL;
    if (inside)
        _eventBus.publish(UIButtonClickedEvent{*pressed, button->action});
}

void UIInteractionSystem::focusNext(SparseArray<UIButton> &buttons)
{
    moveFocus(buttons, true);
}

void UIInteractionSystem::focusPrevious(SparseArray<UIButton> &buttons)
{
    moveFocus(buttons, false);
}

void UIInteractionSystem::activateFocused(SparseArray<UIButton> &buttons)
{
    dropDisabledFocus(buttons);
    const auto focused = focusedEntity(buttons);
    if (!focused.has_value())
        return;
    _eventBus.publish(
        UIButtonClickedEvent{*focused, componentAt(buttons, *focused)->action});
}

void UIInteractionSystem::onKeyPressed(UIKey key,
                                       SparseArray<UIButton> &buttons)
{
    switch (key) {
        case UIKey::TAB:
        case UIKey::DOWN:
        case UIKey::RIGHT:
            focusNext(buttons);
            break;
        case UIKey::UP:
        case UIKey::LEFT:
            focusPrevious(buttons);
            break;
        case UIKey::ENTER:
        case UIKey::SPACE:
            activateFocused(buttons);
            break;
    }
}

std::optional<std::size_t> UIInteractionSystem::pressedEntity() const noexcept
{
    return _pressedEntity;
}

std::optional<std::size_t> UIInteractionSystem::focusedEntity(
    const SparseArray<UIButton> &buttons) noexcept
{
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        const UIButton *button = componentAt(buttons, i);
        if (button != nullptr && isEnabled(*button) && button->isFocused)
            return i;
    }
    return std::nullopt;
}

} // namespace engine::ecs::systems
