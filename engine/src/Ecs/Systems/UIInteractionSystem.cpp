/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIInteractionSystem.cpp
*/

#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Engine/Ecs/IndexedZipper.hpp"
#include "Engine/Ecs/Systems/UIWidgetLogic.hpp"
#include "Engine/Ecs/Zipper.hpp"

namespace engine::ecs::systems {

namespace {

using components::UIButton;
using components::UIButtonClickedEvent;
using components::UIButtonState;
using components::UIDropdown;
using components::UIScrollList;
using components::UISlider;
using components::UITextInput;
using components::UIToggle;
using components::UITransform;

struct FocusSlot {
    std::size_t index;
    bool *focused;
    bool enabled;
};

bool isEnabled(const UIButton &button) noexcept
{
    return button.state != UIButtonState::DISABLED;
}

template <typename Widget>
bool isEnabled(const Widget &widget) noexcept
{
    return widget.enabled;
}

template <typename Widget>
void collectFocus(SparseArray<Widget> &array, std::vector<FocusSlot> &slots)
{
    for (auto [index, widget] : makeIndexedZipper(array))
        slots.push_back({index, &widget.isFocused, isEnabled(widget)});
}

std::vector<FocusSlot> focusSlots(Registry &registry)
{
    std::vector<FocusSlot> slots;

    collectFocus(registry.getComponents<UIButton>(), slots);
    collectFocus(registry.getComponents<UITextInput>(), slots);
    collectFocus(registry.getComponents<UISlider>(), slots);
    collectFocus(registry.getComponents<UIToggle>(), slots);
    collectFocus(registry.getComponents<UIDropdown>(), slots);
    collectFocus(registry.getComponents<UIScrollList>(), slots);
    std::stable_sort(slots.begin(), slots.end(),
                     [](const FocusSlot &lhs, const FocusSlot &rhs) {
                         return lhs.index < rhs.index;
                     });
    return slots;
}

void clearFocus(Registry &registry)
{
    for (const FocusSlot &slot : focusSlots(registry))
        *slot.focused = false;
}

void dropDisabledFocus(Registry &registry)
{
    for (const FocusSlot &slot : focusSlots(registry)) {
        if (!slot.enabled)
            *slot.focused = false;
    }
}

void moveFocus(Registry &registry, bool forward)
{
    std::vector<FocusSlot> enabled = focusSlots(registry);
    std::erase_if(enabled, [](const FocusSlot &slot) { return !slot.enabled; });
    const auto current =
        std::find_if(enabled.begin(), enabled.end(),
                     [](const FocusSlot &slot) { return *slot.focused; });
    const std::size_t count = enabled.size();

    clearFocus(registry);
    if (count == 0)
        return;
    std::size_t next = forward ? 0 : count - 1;
    if (current != enabled.end())
        next = (static_cast<std::size_t>(current - enabled.begin()) +
                (forward ? 1 : count - 1)) %
               count;
    *enabled[next].focused = true;
}

template <typename Widget>
void findFocused(const SparseArray<Widget> &array,
                 std::optional<std::size_t> &best)
{
    for (auto [index, widget] : makeIndexedZipper(array)) {
        if (widget.isFocused && isEnabled(widget) &&
            (!best.has_value() || index < *best))
            best = index;
    }
}

template <typename Widget>
void findHit(const Registry &registry, float x, float y,
             std::optional<std::pair<int, std::size_t>> &best)
{
    for (auto [index, transform, widget] :
         makeIndexedZipper(registry.getComponents<UITransform>(),
                           registry.getComponents<Widget>())) {
        if (isEnabled(widget) &&
            widgets::containsPoint(transform.actualBounds, x, y) &&
            (!best.has_value() || transform.zIndex > best->first))
            best = std::make_pair(transform.zIndex, index);
    }
}

UIButtonState hoverState(bool inside, bool pressed) noexcept
{
    if (pressed)
        return inside ? UIButtonState::PRESSED : UIButtonState::NORMAL;
    return inside ? UIButtonState::HOVERED : UIButtonState::NORMAL;
}

int wheelRows(float delta) noexcept
{
    const auto rows = static_cast<int>(std::lround(delta));

    if (rows != 0)
        return -rows;
    return delta > 0.0f ? -1 : 1;
}

} // namespace

UIInteractionSystem::UIInteractionSystem(core::EventBus &eventBus) noexcept
    : _eventBus(eventBus)
{}

void UIInteractionSystem::onMouseMove(float mouseX, float mouseY)
{
    _pending.push_back({Input::Type::MOVE, mouseX, mouseY});
}

void UIInteractionSystem::onMouseDown(float mouseX, float mouseY)
{
    _pending.push_back({Input::Type::DOWN, mouseX, mouseY});
}

void UIInteractionSystem::onMouseUp(float mouseX, float mouseY)
{
    _pending.push_back({Input::Type::UP, mouseX, mouseY});
}

void UIInteractionSystem::onMouseWheel(float mouseX, float mouseY, float delta)
{
    _pending.push_back({Input::Type::WHEEL, mouseX, mouseY, delta});
}

void UIInteractionSystem::onKeyPressed(UIKey key)
{
    _pending.push_back({Input::Type::KEY, 0.0f, 0.0f, 0.0f, key});
}

void UIInteractionSystem::onTextEntered(char32_t character)
{
    _pending.push_back(
        {Input::Type::TEXT, 0.0f, 0.0f, 0.0f, UIKey::TAB, character});
}

void UIInteractionSystem::discardInput() noexcept
{
    _pending.clear();
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

    access.read<UITransform>()
        .write<UIButton>()
        .write<UITextInput>()
        .write<UISlider>()
        .write<UIToggle>()
        .write<UIDropdown>()
        .write<UIScrollList>();
    return scheduler.addSystem("ui_interaction", phase, access,
                               [this](Registry &registry, CommandBuffer &,
                                      double) { process(registry); });
}

void UIInteractionSystem::apply(const Input &input, Registry &registry)
{
    dropDisabledFocus(registry);
    switch (input.type) {
        case Input::Type::KEY:
            keyPressed(input.key, registry);
            break;
        case Input::Type::TEXT:
            textEntered(input.character, registry);
            break;
        case Input::Type::WHEEL:
            wheelScrolled(input.x, input.y, input.delta, registry);
            break;
        default:
            applyPointer(input, registry);
            break;
    }
}

void UIInteractionSystem::applyPointer(const Input &input, Registry &registry)
{
    if (input.type == Input::Type::MOVE)
        pointerMoved(input.x, input.y, registry);
    else if (input.type == Input::Type::DOWN)
        pointerPressed(input.x, input.y, registry);
    else
        pointerReleased(input.x, input.y, registry);
}

void UIInteractionSystem::pointerMoved(float x, float y, Registry &registry)
{
    hoverButtons(x, y, registry);
    hoverDropdowns(x, y, registry);
    if (_pressed.has_value() && registry.isAlive(*_pressed))
        dragSlider(*_pressed, x, registry);
}

void UIInteractionSystem::hoverButtons(float x, float y, Registry &registry)
{
    auto &buttons = registry.getComponents<UIButton>();
    const auto &transforms =
        std::as_const(registry).getComponents<UITransform>();

    for (auto [index, transform, button] :
         makeIndexedZipper(transforms, buttons)) {
        if (!isEnabled(button))
            continue;
        const bool pressed = _pressed.has_value() && _pressed->index() == index;
        button.state = hoverState(
            widgets::containsPoint(transform.actualBounds, x, y), pressed);
    }
}

void UIInteractionSystem::pointerPressed(float x, float y, Registry &registry)
{
    if (pressOpenDropdowns(x, y, registry))
        return;
    const auto hit = widgetAt(x, y, registry);
    if (!hit.has_value())
        return;
    const Entity entity = registry.entityAt(*hit);
    giveFocus(registry, entity);
    _pressed = entity;
    pressWidget(entity, x, y, registry);
}

void UIInteractionSystem::pointerReleased(float x, float y, Registry &registry)
{
    const auto pressed = std::exchange(_pressed, std::nullopt);

    if (!pressed.has_value() || !registry.isAlive(*pressed))
        return;
    const UITransform *transform =
        std::as_const(registry).getComponents<UITransform>().get(*pressed);
    const bool inside = transform != nullptr &&
                        widgets::containsPoint(transform->actualBounds, x, y);
    releaseButton(*pressed, inside, registry);
    releaseToggle(*pressed, inside, registry);
}

void UIInteractionSystem::wheelScrolled(float x, float y, float delta,
                                        Registry &registry)
{
    const auto &transforms =
        std::as_const(registry).getComponents<UITransform>();

    for (auto [transform, list] :
         makeZipper(transforms, registry.getComponents<UIScrollList>())) {
        if (list.enabled &&
            widgets::containsPoint(transform.actualBounds, x, y))
            widgets::scrollBy(list, wheelRows(delta));
    }
}

void UIInteractionSystem::keyPressed(UIKey key, Registry &registry)
{
    if (claimedByTextInput(key, registry))
        return;
    if (key == UIKey::TAB || key == UIKey::BACK_TAB) {
        if (key == UIKey::TAB)
            focusNext(registry);
        else
            focusPrevious(registry);
        return;
    }
    const auto focused = focusedEntity(registry);
    if (focused.has_value() && handleWidgetKey(*focused, key, registry))
        return;
    navigate(key, registry);
}

void UIInteractionSystem::navigate(UIKey key, Registry &registry)
{
    switch (key) {
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
        case UIKey::ESCAPE:
            _eventBus.publish(components::UICancelEvent{});
            break;
        default:
            break;
    }
}

void UIInteractionSystem::textEntered(char32_t character, Registry &registry)
{
    const auto focused = focusedEntity(registry);

    if (!focused.has_value())
        return;
    UITextInput *input = registry.getComponents<UITextInput>().get(*focused);
    if (input != nullptr && widgets::insertCharacter(*input, character))
        publishText(*focused, *input);
}

void UIInteractionSystem::activateFocused(Registry &registry)
{
    const auto focused = focusedEntity(registry);

    if (!focused.has_value())
        return;
    const UIButton *button =
        std::as_const(registry).getComponents<UIButton>().get(*focused);
    if (button != nullptr)
        _eventBus.publish(
            UIButtonClickedEvent{focused->index(), button->action});
}

void UIInteractionSystem::focusNext(Registry &registry)
{
    moveFocus(registry, true);
    closeDropdowns(registry);
}

void UIInteractionSystem::focusPrevious(Registry &registry)
{
    moveFocus(registry, false);
    closeDropdowns(registry);
}

std::optional<Entity> UIInteractionSystem::pressedEntity() const noexcept
{
    return _pressed;
}

std::optional<Entity>
UIInteractionSystem::focusedEntity(const Registry &registry)
{
    std::optional<std::size_t> best;

    findFocused(registry.getComponents<UIButton>(), best);
    findFocused(registry.getComponents<UITextInput>(), best);
    findFocused(registry.getComponents<UISlider>(), best);
    findFocused(registry.getComponents<UIToggle>(), best);
    findFocused(registry.getComponents<UIDropdown>(), best);
    findFocused(registry.getComponents<UIScrollList>(), best);
    if (!best.has_value())
        return std::nullopt;
    return registry.entityAt(*best);
}

void UIInteractionSystem::giveFocus(Registry &registry, Entity entity)
{
    clearFocus(registry);
    for (const FocusSlot &slot : focusSlots(registry)) {
        if (slot.index == entity.index() && slot.enabled)
            *slot.focused = true;
    }
}

void UIInteractionSystem::closeDropdowns(Registry &registry)
{
    for (auto [dropdown] : makeZipper(registry.getComponents<UIDropdown>())) {
        if (!dropdown.isFocused)
            dropdown.open = false;
    }
}

std::optional<std::size_t>
UIInteractionSystem::widgetAt(float x, float y, const Registry &registry)
{
    std::optional<std::pair<int, std::size_t>> best;

    findHit<UIButton>(registry, x, y, best);
    findHit<UITextInput>(registry, x, y, best);
    findHit<UISlider>(registry, x, y, best);
    findHit<UIToggle>(registry, x, y, best);
    findHit<UIDropdown>(registry, x, y, best);
    findHit<UIScrollList>(registry, x, y, best);
    if (!best.has_value())
        return std::nullopt;
    return best->second;
}

} // namespace engine::ecs::systems
