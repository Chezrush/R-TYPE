/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIWidgetInput.cpp
*/

#include <algorithm>
#include <optional>
#include <utility>

#include "Engine/Ecs/IndexedZipper.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"
#include "Engine/Ecs/Systems/UIWidgetLogic.hpp"
#include "Engine/Ecs/Zipper.hpp"

namespace engine::ecs::systems {

namespace {

using components::UIButton;
using components::UIButtonClickedEvent;
using components::UIButtonState;
using components::UIDropdown;
using components::UIScrollList;
using components::UISelectionChangedEvent;
using components::UISlider;
using components::UITextChangedEvent;
using components::UITextInput;
using components::UIToggle;
using components::UITransform;
using components::UIValueChangedEvent;

template <typename Component>
Component *find(Registry &registry, Entity entity)
{
    return registry.getComponents<Component>().get(entity);
}

const components::Rect *boundsOf(const Registry &registry, Entity entity)
{
    const UITransform *transform =
        registry.getComponents<UITransform>().get(entity);

    return transform == nullptr ? nullptr : &transform->actualBounds;
}

std::size_t stepIndex(std::size_t index, std::size_t count, bool forward)
{
    if (count == 0)
        return 0;
    if (forward)
        return std::min(index + 1, count - 1);
    return index == 0 ? 0 : std::min(index, count) - 1;
}

std::optional<std::size_t> rowForKey(const UIScrollList &list, UIKey key)
{
    const std::size_t count = list.rows.size();

    if (count == 0)
        return std::nullopt;
    switch (key) {
        case UIKey::UP:
        case UIKey::DOWN:
            if (!list.selected.has_value())
                return 0;
            return stepIndex(*list.selected, count, key == UIKey::DOWN);
        case UIKey::HOME:
            return 0;
        case UIKey::END:
            return count - 1;
        default:
            return std::nullopt;
    }
}

} // namespace

void UIInteractionSystem::hoverDropdowns(float x, float y, Registry &registry)
{
    const auto &transforms =
        std::as_const(registry).getComponents<UITransform>();

    for (auto [transform, dropdown] :
         makeZipper(transforms, registry.getComponents<UIDropdown>())) {
        const auto option =
            widgets::dropdownOptionAt(dropdown, transform.actualBounds, x, y);
        if (option.has_value())
            dropdown.highlighted = *option;
    }
}

void UIInteractionSystem::dragSlider(Entity entity, float x, Registry &registry)
{
    auto *slider = find<UISlider>(registry, entity);
    const components::Rect *bounds = boundsOf(registry, entity);

    if (slider == nullptr || bounds == nullptr || !slider->enabled)
        return;
    if (widgets::setSliderValue(*slider,
                                widgets::sliderValueAt(*slider, *bounds, x)))
        publishValue(entity, slider->action, slider->value);
}

void UIInteractionSystem::pressWidget(Entity entity, float x, float y,
                                      Registry &registry)
{
    if (auto *button = find<UIButton>(registry, entity))
        button->state = UIButtonState::PRESSED;
    if (auto *input = find<UITextInput>(registry, entity))
        widgets::moveCaretToEdge(*input, true);
    dragSlider(entity, x, registry);
    if (auto *dropdown = find<UIDropdown>(registry, entity)) {
        dropdown->open = !dropdown->open;
        dropdown->highlighted = dropdown->selected;
    }
    auto *list = find<UIScrollList>(registry, entity);
    const components::Rect *bounds = boundsOf(registry, entity);
    if (list == nullptr || bounds == nullptr)
        return;
    const auto row = widgets::scrollRowAt(*list, *bounds, x, y);
    if (row.has_value())
        selectRow(entity, *list, *row);
}

bool UIInteractionSystem::pressOpenDropdowns(float x, float y,
                                             Registry &registry)
{
    const auto &transforms =
        std::as_const(registry).getComponents<UITransform>();

    for (auto [index, transform, dropdown] :
         makeIndexedZipper(transforms, registry.getComponents<UIDropdown>())) {
        const auto option =
            widgets::dropdownOptionAt(dropdown, transform.actualBounds, x, y);
        if (option.has_value()) {
            selectOption(registry.entityAt(index), dropdown, *option);
            return true;
        }
        if (!widgets::containsPoint(transform.actualBounds, x, y))
            dropdown.open = false;
    }
    return false;
}

void UIInteractionSystem::releaseButton(Entity entity, bool inside,
                                        Registry &registry)
{
    auto *button = find<UIButton>(registry, entity);

    if (button == nullptr || button->state == UIButtonState::DISABLED)
        return;
    button->state = inside ? UIButtonState::HOVERED : UIButtonState::NORMAL;
    if (inside)
        _eventBus.publish(UIButtonClickedEvent{entity.index(), button->action});
}

void UIInteractionSystem::releaseToggle(Entity entity, bool inside,
                                        Registry &registry)
{
    auto *toggle = find<UIToggle>(registry, entity);

    if (toggle != nullptr && toggle->enabled && inside)
        flipToggle(entity, *toggle);
}

bool UIInteractionSystem::handleWidgetKey(Entity entity, UIKey key,
                                          Registry &registry)
{
    if (auto *input = find<UITextInput>(registry, entity))
        return textInputKey(entity, *input, key);
    if (auto *slider = find<UISlider>(registry, entity))
        return sliderKey(entity, *slider, key);
    if (auto *toggle = find<UIToggle>(registry, entity))
        return toggleKey(entity, *toggle, key);
    if (auto *dropdown = find<UIDropdown>(registry, entity))
        return dropdownKey(entity, *dropdown, key);
    if (auto *list = find<UIScrollList>(registry, entity))
        return scrollListKey(entity, *list, key);
    return false;
}

bool UIInteractionSystem::claimedByTextInput(UIKey key, Registry &registry)
{
    const auto focused = focusedEntity(registry);
    const bool claimable = key == UIKey::TAB || key == UIKey::BACK_TAB ||
                           key == UIKey::UP || key == UIKey::DOWN ||
                           key == UIKey::ESCAPE;

    if (!focused.has_value() || !claimable)
        return false;
    const auto *input = find<UITextInput>(registry, *focused);

    if (input == nullptr || !input->claimsNavigationKeys)
        return false;
    _eventBus.publish(UITextKeyEvent{focused->index(), input->action, key});
    return true;
}

bool UIInteractionSystem::textInputKey(Entity entity, UITextInput &input,
                                       UIKey key)
{
    if (key == UIKey::LEFT || key == UIKey::RIGHT) {
        widgets::moveCaret(input, key == UIKey::RIGHT);
        return true;
    }
    if (key == UIKey::HOME || key == UIKey::END) {
        widgets::moveCaretToEdge(input, key == UIKey::END);
        return true;
    }
    if (key == UIKey::BACKSPACE || key == UIKey::DEL) {
        eraseText(entity, input, key);
        return true;
    }
    if (key == UIKey::ENTER)
        _eventBus.publish(
            UITextSubmittedEvent{entity.index(), input.action, input.text});
    return key == UIKey::ENTER || key == UIKey::SPACE;
}

void UIInteractionSystem::eraseText(Entity entity, UITextInput &input,
                                    UIKey key)
{
    const bool erased = key == UIKey::BACKSPACE
                            ? widgets::eraseBeforeCaret(input)
                            : widgets::eraseAfterCaret(input);

    if (erased)
        publishText(entity, input);
}

bool UIInteractionSystem::sliderKey(Entity entity, UISlider &slider, UIKey key)
{
    bool changed = false;

    switch (key) {
        case UIKey::LEFT:
        case UIKey::RIGHT:
            changed = widgets::stepSlider(slider, key == UIKey::RIGHT);
            break;
        case UIKey::HOME:
        case UIKey::END:
            changed = widgets::setSliderValue(
                slider, key == UIKey::END ? slider.max : slider.min);
            break;
        default:
            return false;
    }
    if (changed)
        publishValue(entity, slider.action, slider.value);
    return true;
}

bool UIInteractionSystem::toggleKey(Entity entity, UIToggle &toggle, UIKey key)
{
    if (key != UIKey::ENTER && key != UIKey::SPACE)
        return false;
    flipToggle(entity, toggle);
    return true;
}

bool UIInteractionSystem::dropdownKey(Entity entity, UIDropdown &dropdown,
                                      UIKey key)
{
    if (dropdown.open)
        return openDropdownKey(entity, dropdown, key);
    switch (key) {
        case UIKey::ENTER:
        case UIKey::SPACE:
            dropdown.open = true;
            dropdown.highlighted = dropdown.selected;
            return true;
        case UIKey::LEFT:
        case UIKey::RIGHT:
            selectOption(entity, dropdown,
                         stepIndex(dropdown.selected, dropdown.options.size(),
                                   key == UIKey::RIGHT));
            return true;
        default:
            return false;
    }
}

bool UIInteractionSystem::openDropdownKey(Entity entity, UIDropdown &dropdown,
                                          UIKey key)
{
    switch (key) {
        case UIKey::UP:
        case UIKey::DOWN:
            dropdown.highlighted =
                stepIndex(dropdown.highlighted, dropdown.options.size(),
                          key == UIKey::DOWN);
            return true;
        case UIKey::ENTER:
        case UIKey::SPACE:
            selectOption(entity, dropdown, dropdown.highlighted);
            return true;
        case UIKey::ESCAPE:
        case UIKey::LEFT:
        case UIKey::RIGHT:
            dropdown.open = key != UIKey::ESCAPE;
            return true;
        default:
            return false;
    }
}

bool UIInteractionSystem::scrollListKey(Entity entity, UIScrollList &list,
                                        UIKey key)
{
    const auto row = rowForKey(list, key);

    if (!row.has_value())
        return false;
    selectRow(entity, list, *row);
    return true;
}

void UIInteractionSystem::flipToggle(Entity entity, UIToggle &toggle)
{
    toggle.checked = !toggle.checked;
    publishValue(entity, toggle.action, toggle.checked ? 1.0f : 0.0f);
}

void UIInteractionSystem::selectOption(Entity entity, UIDropdown &dropdown,
                                       std::size_t option)
{
    dropdown.open = false;
    if (option >= dropdown.options.size())
        return;
    dropdown.highlighted = option;
    if (option == dropdown.selected)
        return;
    dropdown.selected = option;
    _eventBus.publish(UISelectionChangedEvent{
        entity.index(), dropdown.action, option, dropdown.options[option]});
}

void UIInteractionSystem::selectRow(Entity entity, UIScrollList &list,
                                    std::size_t row)
{
    if (row >= list.rows.size())
        return;
    widgets::ensureVisible(list, row);
    if (list.selected == row)
        return;
    list.selected = row;
    _eventBus.publish(UISelectionChangedEvent{entity.index(), list.action, row,
                                              list.rows[row]});
}

void UIInteractionSystem::publishText(Entity entity, const UITextInput &input)
{
    _eventBus.publish(
        UITextChangedEvent{entity.index(), input.action, input.text});
}

void UIInteractionSystem::publishValue(Entity entity, const std::string &action,
                                       float value)
{
    _eventBus.publish(UIValueChangedEvent{entity.index(), action, value});
}

} // namespace engine::ecs::systems
