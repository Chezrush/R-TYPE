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
#include <vector>

#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/Phase.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"

namespace engine::ecs::systems {

/**
 * @brief Keys supported for keyboard/gamepad UI navigation.
 */
enum class UIKey : std::uint8_t {
    TAB,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    ENTER,
    SPACE
};

/**
 * @brief Turns pointer and key input into button states and click events.
 *
 * The on*() methods only queue the input, in arrival order; process() applies
 * the queue to the UITransform and UIButton components of a registry (both
 * must be registered, see registerUIComponents()). Registered with addTo(),
 * process() runs as a scheduler system.
 *
 * Focus rules: at most one button is focused, and a DISABLED button is never
 * focused. Clicking a button gives it the focus. A button disabled by game
 * code loses its focus at the next processed input. A button disabled while
 * pressed never emits a click.
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

    /** @brief Queues a keyboard / gamepad navigation key. */
    void onKeyPressed(UIKey key);

    /** @brief Applies the queued input, in order, then empties the queue. */
    void process(Registry &registry);

    /**
     * @brief Registers process() as a scheduler system.
     *
     * Declared access: reads UITransform, writes UIButton. This object must
     * outlive the scheduler entry. PRE_UPDATE by default, where input goes.
     */
    Scheduler::SystemId addTo(Scheduler &scheduler,
                              Phase phase = Phase::PRE_UPDATE);

    /** @brief Advances focus to the next enabled button. */
    static void focusNext(Registry &registry);

    /** @brief Moves focus to the previous enabled button. */
    static void focusPrevious(Registry &registry);

    /** @brief Button currently held down by the pointer (if any). */
    [[nodiscard]] std::optional<Entity> pressedEntity() const noexcept;

    /** @brief Focused enabled button (if any). */
    [[nodiscard]] static std::optional<Entity>
    focusedEntity(const Registry &registry);

private:
    struct Input {
        enum class Type : std::uint8_t {
            MOVE,
            DOWN,
            UP,
            KEY
        };

        Type type;
        float x;
        float y;
        UIKey key;
    };

    void apply(const Input &input, Registry &registry);
    void pointerMoved(float x, float y, Registry &registry);
    void pointerPressed(float x, float y, Registry &registry);
    void pointerReleased(float x, float y, Registry &registry);
    void keyPressed(UIKey key, Registry &registry);
    void activateFocused(Registry &registry);

    static bool contains(const components::Rect &rect, float x,
                         float y) noexcept;

    /** @brief Index of the first enabled button containing the point. */
    static std::optional<std::size_t> buttonAt(float x, float y,
                                               const Registry &registry);

    core::EventBus &_eventBus;
    std::vector<Input> _pending;
    std::optional<Entity> _pressed;
};

} // namespace engine::ecs::systems

namespace engine::ecs {

using systems::UIInteractionSystem;
using systems::UIKey;

} // namespace engine::ecs

#endif /* !UIINTERACTIONSYSTEM_HPP_ */
