/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIWidgetLogic.cpp
*/

#include "Engine/Ecs/Systems/UIWidgetLogic.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace engine::ecs::systems::widgets {

namespace {

using components::Rect;
using components::UIDropdown;
using components::UIScrollList;
using components::UISlider;
using components::UITextFilter;
using components::UITextInput;

constexpr char32_t MAX_CODE_POINT = 0x10FFFF;
constexpr char32_t SURROGATE_FIRST = 0xD800;
constexpr char32_t SURROGATE_LAST = 0xDFFF;

bool isContinuation(char byte) noexcept
{
    return (static_cast<unsigned char>(byte) & 0xC0U) == 0x80U;
}

bool isAsciiAlphanumeric(char32_t character) noexcept
{
    return (character >= U'0' && character <= U'9') ||
           (character >= U'a' && character <= U'z') ||
           (character >= U'A' && character <= U'Z');
}

bool isControl(char32_t character) noexcept
{
    return character < 0x20 || (character >= 0x7F && character < 0xA0);
}

bool isValidCodePoint(char32_t character) noexcept
{
    return character <= MAX_CODE_POINT &&
           (character < SURROGATE_FIRST || character > SURROGATE_LAST);
}

char utf8Byte(char32_t bits) noexcept
{
    return static_cast<char>(static_cast<unsigned char>(bits));
}

void fixCaret(UITextInput &input) noexcept
{
    input.caret = std::min(input.caret, input.text.size());
    while (input.caret > 0 && input.caret < input.text.size() &&
           isContinuation(input.text[input.caret]))
        --input.caret;
}

std::size_t maxScrollOffset(const UIScrollList &list) noexcept
{
    const std::size_t visible = visibleRowCount(list);

    return list.rows.size() > visible ? list.rows.size() - visible : 0;
}

} // namespace

bool containsPoint(const Rect &rect, float x, float y) noexcept
{
    return x >= rect.x && x <= rect.x + rect.width && y >= rect.y &&
           y <= rect.y + rect.height;
}

bool acceptsCharacter(UITextFilter filter, char32_t character) noexcept
{
    switch (filter) {
        case UITextFilter::DIGITS:
            return character >= U'0' && character <= U'9';
        case UITextFilter::ALPHANUMERIC:
            return isAsciiAlphanumeric(character);
        default:
            return isValidCodePoint(character) && !isControl(character);
    }
}

std::string encodeUtf8(char32_t character)
{
    if (!isValidCodePoint(character))
        return {};
    if (character < 0x80)
        return {utf8Byte(character)};
    if (character < 0x800)
        return {utf8Byte(0xC0 | (character >> 6)),
                utf8Byte(0x80 | (character & 0x3F))};
    if (character < 0x10000)
        return {utf8Byte(0xE0 | (character >> 12)),
                utf8Byte(0x80 | ((character >> 6) & 0x3F)),
                utf8Byte(0x80 | (character & 0x3F))};
    return {utf8Byte(0xF0 | (character >> 18)),
            utf8Byte(0x80 | ((character >> 12) & 0x3F)),
            utf8Byte(0x80 | ((character >> 6) & 0x3F)),
            utf8Byte(0x80 | (character & 0x3F))};
}

std::size_t characterCount(const std::string &text) noexcept
{
    return characterIndex(text, text.size());
}

std::size_t characterIndex(const std::string &text,
                           std::size_t byteOffset) noexcept
{
    const auto end = text.begin() + static_cast<std::ptrdiff_t>(
                                        std::min(byteOffset, text.size()));

    return static_cast<std::size_t>(std::count_if(
        text.begin(), end, [](char byte) { return !isContinuation(byte); }));
}

bool insertCharacter(UITextInput &input, char32_t character)
{
    const std::string bytes = encodeUtf8(character);

    if (bytes.empty() || !acceptsCharacter(input.filter, character) ||
        characterCount(input.text) >= input.maxLength)
        return false;
    fixCaret(input);
    input.text.insert(input.caret, bytes);
    input.caret += bytes.size();
    return true;
}

bool eraseBeforeCaret(UITextInput &input)
{
    fixCaret(input);
    if (input.caret == 0)
        return false;
    std::size_t start = input.caret - 1;
    while (start > 0 && isContinuation(input.text[start]))
        --start;
    input.text.erase(start, input.caret - start);
    input.caret = start;
    return true;
}

bool eraseAfterCaret(UITextInput &input)
{
    fixCaret(input);
    if (input.caret >= input.text.size())
        return false;
    std::size_t end = input.caret + 1;
    while (end < input.text.size() && isContinuation(input.text[end]))
        ++end;
    input.text.erase(input.caret, end - input.caret);
    return true;
}

void moveCaret(UITextInput &input, bool forward) noexcept
{
    fixCaret(input);
    if (forward && input.caret < input.text.size()) {
        ++input.caret;
        while (input.caret < input.text.size() &&
               isContinuation(input.text[input.caret]))
            ++input.caret;
    } else if (!forward && input.caret > 0) {
        --input.caret;
        fixCaret(input);
    }
}

void moveCaretToEdge(UITextInput &input, bool end) noexcept
{
    input.caret = end ? input.text.size() : 0;
}

float snapValue(const UISlider &slider, float value) noexcept
{
    if (slider.max <= slider.min)
        return slider.min;
    float snapped = std::clamp(value, slider.min, slider.max);
    if (slider.step > 0.0f)
        snapped =
            slider.min +
            std::round((snapped - slider.min) / slider.step) * slider.step;
    return std::clamp(snapped, slider.min, slider.max);
}

float sliderRatio(const UISlider &slider) noexcept
{
    if (slider.max <= slider.min)
        return 0.0f;
    return std::clamp((slider.value - slider.min) / (slider.max - slider.min),
                      0.0f, 1.0f);
}

float sliderValueAt(const UISlider &slider, const Rect &bounds,
                    float x) noexcept
{
    const float ratio =
        bounds.width > 0.0f
            ? std::clamp((x - bounds.x) / bounds.width, 0.0f, 1.0f)
            : 0.0f;

    return snapValue(slider, slider.min + ratio * (slider.max - slider.min));
}

bool setSliderValue(UISlider &slider, float value) noexcept
{
    const float snapped = snapValue(slider, value);

    if (snapped == slider.value)
        return false;
    slider.value = snapped;
    return true;
}

bool stepSlider(UISlider &slider, bool increase) noexcept
{
    const float step =
        slider.step > 0.0f ? slider.step : (slider.max - slider.min) / 100.0f;

    return setSliderValue(slider, slider.value + (increase ? step : -step));
}

Rect dropdownOptionBounds(const Rect &bounds, std::size_t option) noexcept
{
    return {bounds.x, bounds.y + bounds.height * static_cast<float>(option + 1),
            bounds.width, bounds.height};
}

std::optional<std::size_t> dropdownOptionAt(const UIDropdown &dropdown,
                                            const Rect &bounds, float x,
                                            float y) noexcept
{
    const float top = bounds.y + bounds.height;

    if (!dropdown.open || bounds.height <= 0.0f || y < top || x < bounds.x ||
        x > bounds.x + bounds.width)
        return std::nullopt;
    const auto option =
        static_cast<std::size_t>(std::floor((y - top) / bounds.height));
    if (option >= dropdown.options.size())
        return std::nullopt;
    return option;
}

std::size_t visibleRowCount(const UIScrollList &list) noexcept
{
    return std::max<std::size_t>(1, list.visibleRows);
}

Rect scrollRowBounds(const UIScrollList &list, const Rect &bounds,
                     std::size_t visibleRow) noexcept
{
    const float rowHeight =
        bounds.height / static_cast<float>(visibleRowCount(list));

    return {bounds.x, bounds.y + rowHeight * static_cast<float>(visibleRow),
            bounds.width, rowHeight};
}

std::optional<std::size_t> scrollRowAt(const UIScrollList &list,
                                       const Rect &bounds, float x,
                                       float y) noexcept
{
    const float rowHeight =
        bounds.height / static_cast<float>(visibleRowCount(list));

    if (!containsPoint(bounds, x, y) || rowHeight <= 0.0f)
        return std::nullopt;
    const std::size_t visible =
        std::min(static_cast<std::size_t>((y - bounds.y) / rowHeight),
                 visibleRowCount(list) - 1);
    const std::size_t row = list.scrollOffset + visible;
    if (row >= list.rows.size())
        return std::nullopt;
    return row;
}

bool scrollBy(UIScrollList &list, int rows) noexcept
{
    const auto target = static_cast<std::int64_t>(list.scrollOffset) + rows;
    const auto clamped = static_cast<std::size_t>(std::clamp<std::int64_t>(
        target, 0, static_cast<std::int64_t>(maxScrollOffset(list))));

    if (clamped == list.scrollOffset)
        return false;
    list.scrollOffset = clamped;
    return true;
}

void ensureVisible(UIScrollList &list, std::size_t row) noexcept
{
    const std::size_t visible = visibleRowCount(list);

    if (row < list.scrollOffset)
        list.scrollOffset = row;
    else if (row >= list.scrollOffset + visible)
        list.scrollOffset = row - visible + 1;
    list.scrollOffset = std::min(list.scrollOffset, maxScrollOffset(list));
}

} // namespace engine::ecs::systems::widgets
