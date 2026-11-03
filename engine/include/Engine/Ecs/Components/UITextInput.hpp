/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UITextInput.hpp
*/

#ifndef UITEXTINPUT_HPP_
#define UITEXTINPUT_HPP_

#include <cstddef>
#include <cstdint>
#include <string>

#include "Engine/Ecs/Components/UIWidgetStyle.hpp"

namespace engine::ecs::components {

/**
 * @brief Characters a text input accepts.
 *
 * PRINTABLE: any character except control characters. ALPHANUMERIC: ASCII
 * letters and digits. DIGITS: 0 to 9.
 */
enum class UITextFilter : std::uint8_t {
    PRINTABLE,
    ALPHANUMERIC,
    DIGITS
};

/**
 * @brief Single-line editable text field.
 *
 * text is UTF-8. caret is a byte offset in text, always at the start of a
 * character. maxLength counts characters, not bytes. The placeholder is shown
 * while the text is empty and the field is not focused.
 */
struct UITextInput {
    std::string text;
    std::string placeholder;
    std::size_t maxLength = 32;
    UITextFilter filter = UITextFilter::PRINTABLE;
    std::size_t caret = 0;
    bool isFocused = false;
    bool enabled = true;
    std::string action;
    UIWidgetStyle style;

    friend bool operator==(const UITextInput &, const UITextInput &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UITextFilter;
using components::UITextInput;

} // namespace engine::ecs

#endif /* !UITEXTINPUT_HPP_ */
