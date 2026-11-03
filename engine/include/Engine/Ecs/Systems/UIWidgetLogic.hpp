/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIWidgetLogic.hpp
*/

#ifndef UIWIDGETLOGIC_HPP_
#define UIWIDGETLOGIC_HPP_

#include <cstddef>
#include <optional>
#include <string>

#include "Engine/Ecs/Components/UIDropdown.hpp"
#include "Engine/Ecs/Components/UIScrollList.hpp"
#include "Engine/Ecs/Components/UISlider.hpp"
#include "Engine/Ecs/Components/UITextInput.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"

/**
 * @brief Rules of the widgets, shared by input handling and rendering.
 *
 * Pure functions on the widget components: no registry, no window. The
 * functions that modify a widget return true when something changed, so the
 * caller knows when to publish an event.
 */
namespace engine::ecs::systems::widgets {

/** @brief Whether the point lies in the rectangle (edges included). */
[[nodiscard]] bool containsPoint(const components::Rect &rect, float x,
                                 float y) noexcept;

/** @brief Whether a text input with this filter accepts the character. */
[[nodiscard]] bool acceptsCharacter(components::UITextFilter filter,
                                    char32_t character) noexcept;

/** @brief UTF-8 bytes of a character, empty for an invalid code point. */
[[nodiscard]] std::string encodeUtf8(char32_t character);

/** @brief Number of characters (not bytes) of a UTF-8 string. */
[[nodiscard]] std::size_t characterCount(const std::string &text) noexcept;

/** @brief Number of characters before a byte offset of a UTF-8 string. */
[[nodiscard]] std::size_t characterIndex(const std::string &text,
                                         std::size_t byteOffset) noexcept;

/** @brief Inserts at the caret if the filter and the max length allow it. */
bool insertCharacter(components::UITextInput &input, char32_t character);

/** @brief Erases the character before the caret (Backspace). */
bool eraseBeforeCaret(components::UITextInput &input);

/** @brief Erases the character after the caret (Delete). */
bool eraseAfterCaret(components::UITextInput &input);

/** @brief Moves the caret by one character. */
void moveCaret(components::UITextInput &input, bool forward) noexcept;

/** @brief Puts the caret at the start or the end of the text. */
void moveCaretToEdge(components::UITextInput &input, bool end) noexcept;

/** @brief Value clamped to [min, max] and put on a step. */
[[nodiscard]] float snapValue(const components::UISlider &slider,
                              float value) noexcept;

/** @brief Position of the value in the range, in [0, 1]. */
[[nodiscard]] float sliderRatio(const components::UISlider &slider) noexcept;

/** @brief Value under a horizontal position of the slider bounds. */
[[nodiscard]] float sliderValueAt(const components::UISlider &slider,
                                  const components::Rect &bounds,
                                  float x) noexcept;

/** @brief Sets the snapped value. */
bool setSliderValue(components::UISlider &slider, float value) noexcept;

/** @brief Moves by one step (1 % of the range for a continuous slider). */
bool stepSlider(components::UISlider &slider, bool increase) noexcept;

/** @brief Row of an option of an open dropdown, below its bounds. */
[[nodiscard]] components::Rect
dropdownOptionBounds(const components::Rect &bounds,
                     std::size_t option) noexcept;

/** @brief Option of an open dropdown under the point, if any. */
[[nodiscard]] std::optional<std::size_t>
dropdownOptionAt(const components::UIDropdown &dropdown,
                 const components::Rect &bounds, float x, float y) noexcept;

/** @brief Number of rows shown at once, at least 1. */
[[nodiscard]] std::size_t
visibleRowCount(const components::UIScrollList &list) noexcept;

/** @brief Rectangle of a visible row (0 is the top one). */
[[nodiscard]] components::Rect
scrollRowBounds(const components::UIScrollList &list,
                const components::Rect &bounds,
                std::size_t visibleRow) noexcept;

/** @brief Row (absolute index) under the point, if any. */
[[nodiscard]] std::optional<std::size_t>
scrollRowAt(const components::UIScrollList &list,
            const components::Rect &bounds, float x, float y) noexcept;

/** @brief Scrolls by a number of rows, kept inside the list. */
bool scrollBy(components::UIScrollList &list, int rows) noexcept;

/** @brief Scrolls the least possible so that the row is visible. */
void ensureVisible(components::UIScrollList &list, std::size_t row) noexcept;

} // namespace engine::ecs::systems::widgets

#endif /* !UIWIDGETLOGIC_HPP_ */
