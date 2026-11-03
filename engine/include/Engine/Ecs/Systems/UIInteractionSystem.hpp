/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIInteractionSystem.hpp
*/

#ifndef UIINTERACTIONSYSTEM_HPP_
#define UIINTERACTIONSYSTEM_HPP_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UIDropdown.hpp"
#include "Engine/Ecs/Components/UIScrollList.hpp"
#include "Engine/Ecs/Components/UISlider.hpp"
#include "Engine/Ecs/Components/UITextInput.hpp"
#include "Engine/Ecs/Components/UIToggle.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/Phase.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"

namespace engine::ecs::systems {

/**
 * @brief Keys of the UI, independent of any window library.
 *
 * Map the keyboard and the gamepad onto them: for example Tab / RB -> TAB,
 * Shift+Tab / LB -> BACK_TAB, D-pad -> arrows, A -> ENTER, B -> ESCAPE.
 */
enum class UIKey : std::uint8_t {
    TAB,
    BACK_TAB,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    ENTER,
    SPACE,
    ESCAPE,
    BACKSPACE,
    DEL,
    HOME,
    END
};

/**
 * @brief A navigation key pressed in a text input that claims those keys.
 *
 * See UITextInput::claimsNavigationKeys.
 */
struct UITextKeyEvent {
    std::size_t entityIndex = 0;
    std::string action;
    UIKey key = UIKey::TAB;

    friend bool operator==(const UITextKeyEvent &,
                           const UITextKeyEvent &) = default;
};

/**
 * @brief Turns pointer, key and text input into widget changes and events.
 *
 * The on*() methods only queue the input, in arrival order; process() applies
 * the queue to the UI components of a registry (see registerUIComponents()).
 * Registered with addTo(), process() runs as a scheduler system.
 *
 * Focus rules: at most one widget (button, text input, slider, toggle,
 * dropdown, scroll list) is focused, and a disabled one never is. TAB and
 * BACK_TAB cycle through all of them in entity order; clicking one gives it
 * the focus. The focused widget receives the keys first (arrows move a caret,
 * a slider or a selection); the keys it does not use navigate (Down / Right
 * next, Up / Left previous, Enter / Space press the focused button). A widget
 * disabled by game code loses its focus at the next processed input.
 *
 * Every change is published on the event bus: UIButtonClickedEvent,
 * UITextChangedEvent, UIValueChangedEvent, UISelectionChangedEvent. Enter in
 * a text input publishes UITextSubmittedEvent; Escape, when the focused widget
 * does not use it, publishes UICancelEvent.
 */
class UIInteractionSystem {
public:
    explicit UIInteractionSystem(core::EventBus &eventBus) noexcept;

    /** @brief Queues a pointer move. */
    void onMouseMove(float mouseX, float mouseY);

    /** @brief Queues a pointer press. */
    void onMouseDown(float mouseX, float mouseY);

    /** @brief Queues a pointer release (a click if over the pressed button). */
    void onMouseUp(float mouseX, float mouseY);

    /** @brief Queues a wheel move over a point, positive is upwards. */
    void onMouseWheel(float mouseX, float mouseY, float delta);

    /** @brief Queues a keyboard / gamepad key. */
    void onKeyPressed(UIKey key);

    /** @brief Queues a typed character, for the focused text input. */
    void onTextEntered(char32_t character);

    /** @brief Forgets the queued input, for a modal that reads devices itself.
     */
    void discardInput() noexcept;

    /** @brief Applies the queued input, in order, then empties the queue. */
    void process(Registry &registry);

    /**
     * @brief Registers process() as a scheduler system.
     *
     * Declared access: reads UITransform, writes the interactive widgets.
     * This object must outlive the scheduler entry. PRE_UPDATE by default,
     * where input goes.
     */
    Scheduler::SystemId addTo(Scheduler &scheduler,
                              Phase phase = Phase::PRE_UPDATE);

    /** @brief Advances focus to the next enabled widget. */
    static void focusNext(Registry &registry);

    /** @brief Moves focus to the previous enabled widget. */
    static void focusPrevious(Registry &registry);

    /** @brief Widget currently held down by the pointer (if any). */
    [[nodiscard]] std::optional<Entity> pressedEntity() const noexcept;

    /** @brief Focused enabled widget (if any). */
    [[nodiscard]] static std::optional<Entity>
    focusedEntity(const Registry &registry);

private:
    struct Input {
        enum class Type : std::uint8_t {
            MOVE,
            DOWN,
            UP,
            WHEEL,
            KEY,
            TEXT
        };

        Type type;
        float x = 0.0f;
        float y = 0.0f;
        float delta = 0.0f;
        UIKey key = UIKey::TAB;
        char32_t character = 0;
    };

    void apply(const Input &input, Registry &registry);
    void applyPointer(const Input &input, Registry &registry);
    void pointerMoved(float x, float y, Registry &registry);
    void pointerPressed(float x, float y, Registry &registry);
    void pointerReleased(float x, float y, Registry &registry);
    static void wheelScrolled(float x, float y, float delta,
                              Registry &registry);
    void keyPressed(UIKey key, Registry &registry);
    bool claimedByTextInput(UIKey key, Registry &registry);
    void navigate(UIKey key, Registry &registry);
    void textEntered(char32_t character, Registry &registry);
    void activateFocused(Registry &registry);
    void hoverButtons(float x, float y, Registry &registry);

    static void hoverDropdowns(float x, float y, Registry &registry);
    void dragSlider(Entity entity, float x, Registry &registry);
    void pressWidget(Entity entity, float x, float y, Registry &registry);
    bool pressOpenDropdowns(float x, float y, Registry &registry);
    void releaseButton(Entity entity, bool inside, Registry &registry);
    void releaseToggle(Entity entity, bool inside, Registry &registry);
    bool handleWidgetKey(Entity entity, UIKey key, Registry &registry);
    bool textInputKey(Entity entity, components::UITextInput &input, UIKey key);
    void eraseText(Entity entity, components::UITextInput &input, UIKey key);
    bool sliderKey(Entity entity, components::UISlider &slider, UIKey key);
    bool toggleKey(Entity entity, components::UIToggle &toggle, UIKey key);
    bool dropdownKey(Entity entity, components::UIDropdown &dropdown,
                     UIKey key);
    bool openDropdownKey(Entity entity, components::UIDropdown &dropdown,
                         UIKey key);
    bool scrollListKey(Entity entity, components::UIScrollList &list,
                       UIKey key);
    void flipToggle(Entity entity, components::UIToggle &toggle);
    void selectOption(Entity entity, components::UIDropdown &dropdown,
                      std::size_t option);
    void selectRow(Entity entity, components::UIScrollList &list,
                   std::size_t row);
    void publishText(Entity entity, const components::UITextInput &input);
    void publishValue(Entity entity, const std::string &action, float value);

    static void giveFocus(Registry &registry, Entity entity);
    static void closeDropdowns(Registry &registry);

    /** @brief Index of the topmost enabled widget containing the point. */
    static std::optional<std::size_t> widgetAt(float x, float y,
                                               const Registry &registry);

    core::EventBus &_eventBus;
    std::vector<Input> _pending;
    std::optional<Entity> _pressed;
};

} // namespace engine::ecs::systems

namespace engine::ecs {

using systems::UIInteractionSystem;
using systems::UIKey;
using systems::UITextKeyEvent;

} // namespace engine::ecs

#endif /* !UIINTERACTIONSYSTEM_HPP_ */
