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

/** @brief Slot at a raw index, through the public iterators. */
template <typename Component>
typename SparseArray<Component>::Slot &slotAt(
    SparseArray<Component> &array,
    std::size_t index
) noexcept
{
    return *(array.begin() + static_cast<std::ptrdiff_t>(index));
}

template <typename Component>
const typename SparseArray<Component>::Slot &slotAt(
    const SparseArray<Component> &array,
    std::size_t index
) noexcept
{
    return *(array.begin() + static_cast<std::ptrdiff_t>(index));
}

void updateButtonHover(
    components::UIButton &btn,
    bool inside,
    bool isPressedEntity
) noexcept
{
    using components::UIButtonState;
    if (isPressedEntity) {
        btn.state = inside ? UIButtonState::PRESSED : UIButtonState::HOVERED;
    } else if (btn.state != UIButtonState::PRESSED) {
        btn.state = inside ? UIButtonState::HOVERED : UIButtonState::NORMAL;
    }
}

std::vector<std::size_t> findEnabledButtons(
    const SparseArray<components::UIButton> &buttons
)
{
    std::vector<std::size_t> enabled;
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        if (slotAt(buttons, i).has_value() &&
            slotAt(buttons, i)->state != components::UIButtonState::DISABLED) {
            enabled.push_back(i);
        }
    }
    return enabled;
}

} // namespace

UIInteractionSystem::UIInteractionSystem(core::EventBus &eventBus) noexcept
    : _eventBus(eventBus)
{}

bool UIInteractionSystem::contains(
    const components::Rect &rect,
    float x,
    float y
) noexcept
{
    return x >= rect.x && x <= (rect.x + rect.width) &&
           y >= rect.y && y <= (rect.y + rect.height);
}

void UIInteractionSystem::onMouseMove(
    float mouseX,
    float mouseY,
    const SparseArray<components::UITransform> &transforms,
    SparseArray<components::UIButton> &buttons
)
{
    const std::size_t count = std::min(transforms.size(), buttons.size());
    for (std::size_t i = 0; i < count; ++i) {
        const auto &transform = slotAt(transforms, i);
        auto &button = slotAt(buttons, i);
        if (transform.has_value() && button.has_value()) {
            auto &btn = *button;
            if (btn.state != components::UIButtonState::DISABLED) {
                const bool inside =
                    contains(transform->actualBounds, mouseX, mouseY);
                const bool isPressed =
                    _pressedEntity.has_value() && _pressedEntity.value() == i;
                updateButtonHover(btn, inside, isPressed);
            }
        }
    }
}

void UIInteractionSystem::onMouseDown(
    float mouseX,
    float mouseY,
    const SparseArray<components::UITransform> &transforms,
    SparseArray<components::UIButton> &buttons
)
{
    const std::size_t count = std::min(transforms.size(), buttons.size());
    for (std::size_t i = 0; i < count; ++i) {
        const auto &transform = slotAt(transforms, i);
        auto &button = slotAt(buttons, i);
        if (transform.has_value() && button.has_value()) {
            auto &btn = *button;
            if (btn.state == components::UIButtonState::DISABLED) {
                continue;
            }
            if (contains(transform->actualBounds, mouseX, mouseY)) {
                btn.state = components::UIButtonState::PRESSED;
                _pressedEntity = i;
                btn.isFocused = true;
                return;
            }
        }
    }
}

void UIInteractionSystem::onMouseUp(
    float mouseX,
    float mouseY,
    const SparseArray<components::UITransform> &transforms,
    SparseArray<components::UIButton> &buttons
)
{
    if (!_pressedEntity.has_value()) {
        return;
    }
    const std::size_t idx = _pressedEntity.value();
    _pressedEntity.reset();
    if (idx < buttons.size() && slotAt(buttons, idx).has_value()) {
        auto &btn = *slotAt(buttons, idx);
        const bool inside = idx < transforms.size() &&
            slotAt(transforms, idx).has_value() &&
            contains(slotAt(transforms, idx)->actualBounds, mouseX, mouseY);
        btn.state = inside ? components::UIButtonState::HOVERED
                           : components::UIButtonState::NORMAL;
        if (inside && btn.state != components::UIButtonState::DISABLED) {
            _eventBus.publish(
                components::UIButtonClickedEvent{idx, btn.action});
        }
    }
}

void UIInteractionSystem::focusNext(
    SparseArray<components::UIButton> &buttons
)
{
    const auto enabled = findEnabledButtons(buttons);
    if (enabled.empty()) {
        return;
    }
    std::size_t nextPos = 0;
    for (std::size_t i = 0; i < enabled.size(); ++i) {
        if (slotAt(buttons, enabled[i])->isFocused) {
            nextPos = (i + 1) % enabled.size();
            slotAt(buttons, enabled[i])->isFocused = false;
            break;
        }
    }
    slotAt(buttons, enabled[nextPos])->isFocused = true;
}

void UIInteractionSystem::focusPrevious(
    SparseArray<components::UIButton> &buttons
)
{
    const auto enabled = findEnabledButtons(buttons);
    if (enabled.empty()) {
        return;
    }
    std::size_t prevPos = enabled.size() - 1;
    for (std::size_t i = 0; i < enabled.size(); ++i) {
        if (slotAt(buttons, enabled[i])->isFocused) {
            prevPos = (i + enabled.size() - 1) % enabled.size();
            slotAt(buttons, enabled[i])->isFocused = false;
            break;
        }
    }
    slotAt(buttons, enabled[prevPos])->isFocused = true;
}

void UIInteractionSystem::activateFocused(
    SparseArray<components::UIButton> &buttons
)
{
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        const auto &button = slotAt(buttons, i);
        if (button.has_value() && button->isFocused &&
            button->state != components::UIButtonState::DISABLED) {
            _eventBus.publish(
                components::UIButtonClickedEvent{i, button->action});
            return;
        }
    }
}

void UIInteractionSystem::onKeyPressed(
    UIKey key,
    SparseArray<components::UIButton> &buttons
)
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
    const SparseArray<components::UIButton> &buttons
) const noexcept
{
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        if (slotAt(buttons, i).has_value() && slotAt(buttons, i)->isFocused) {
            return i;
        }
    }
    return std::nullopt;
}

} // namespace engine::ecs::systems

