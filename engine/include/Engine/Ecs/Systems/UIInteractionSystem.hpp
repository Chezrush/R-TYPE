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

#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/SparseArray.hpp"

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
 * @brief System that processes mouse/keyboard interactions and emits events.
 *
 * Focus rules: at most one button is focused, and a DISABLED button is never
 * focused. Clicking a button gives it the focus. A button disabled by game
 * code loses its focus at the next call to this system. A button disabled
 * while pressed never emits a click.
 */
class UIInteractionSystem {
public:
    explicit UIInteractionSystem(core::EventBus &eventBus) noexcept;

    /** @brief Handles mouse movement over UI elements. */
    void onMouseMove(float mouseX, float mouseY,
                     const SparseArray<components::UITransform> &transforms,
                     SparseArray<components::UIButton> &buttons);

    /** @brief Handles mouse button press. */
    void onMouseDown(float mouseX, float mouseY,
                     const SparseArray<components::UITransform> &transforms,
                     SparseArray<components::UIButton> &buttons);

    /** @brief Handles mouse button release. Triggers click if over button. */
    void onMouseUp(float mouseX, float mouseY,
                   const SparseArray<components::UITransform> &transforms,
                   SparseArray<components::UIButton> &buttons);

    /** @brief Handles keyboard/gamepad key navigation. */
    void onKeyPressed(UIKey key, SparseArray<components::UIButton> &buttons);

    /** @brief Advances focus to the next enabled button. */
    static void focusNext(SparseArray<components::UIButton> &buttons);

    /** @brief Moves focus to the previous enabled button. */
    static void focusPrevious(SparseArray<components::UIButton> &buttons);

    /** @brief Activates the currently focused button, emitting click event. */
    void activateFocused(SparseArray<components::UIButton> &buttons);

    /** @brief Returns index of currently pressed button (if any). */
    [[nodiscard]] std::optional<std::size_t> pressedEntity() const noexcept;

    /** @brief Returns index of the focused enabled button (if any). */
    [[nodiscard]] static std::optional<std::size_t>
    focusedEntity(const SparseArray<components::UIButton> &buttons) noexcept;

private:
    static bool contains(const components::Rect &rect, float x,
                         float y) noexcept;

    /** @brief First enabled button whose bounds contain the point. */
    static std::optional<std::size_t>
    buttonAt(float x, float y,
             const SparseArray<components::UITransform> &transforms,
             const SparseArray<components::UIButton> &buttons);

    core::EventBus &_eventBus;
    std::optional<std::size_t> _pressedEntity;
};

} // namespace engine::ecs::systems

namespace engine::ecs {

using systems::UIInteractionSystem;
using systems::UIKey;

} // namespace engine::ecs

#endif /* !UIINTERACTIONSYSTEM_HPP_ */
