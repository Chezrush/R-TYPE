/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIWidgetLogic.cpp
*/

#include "Engine/Ecs/Systems/UIWidgetLogic.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

using engine::ecs::components::Rect;
using engine::ecs::components::UIDropdown;
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UISlider;
using engine::ecs::components::UITextFilter;
using engine::ecs::components::UITextInput;
namespace widgets = engine::ecs::systems::widgets;

namespace {

UITextInput inputWith(std::string text, std::size_t caret)
{
    UITextInput input;

    input.text = std::move(text);
    input.caret = caret;
    return input;
}

UIScrollList listOf(std::size_t rows, std::size_t visible)
{
    UIScrollList list;

    list.rows.assign(rows, "row");
    list.visibleRows = visible;
    return list;
}

} // namespace

TEST_CASE("text filters accept the right characters", "[ui][widgets][text]")
{
    REQUIRE(widgets::acceptsCharacter(UITextFilter::DIGITS, U'7'));
    REQUIRE_FALSE(widgets::acceptsCharacter(UITextFilter::DIGITS, U'a'));
    REQUIRE(widgets::acceptsCharacter(UITextFilter::ALPHANUMERIC, U'Z'));
    REQUIRE_FALSE(widgets::acceptsCharacter(UITextFilter::ALPHANUMERIC, U'-'));
    REQUIRE_FALSE(
        widgets::acceptsCharacter(UITextFilter::ALPHANUMERIC, U'\u00E9'));
    REQUIRE(widgets::acceptsCharacter(UITextFilter::PRINTABLE, U'\u00E9'));
    REQUIRE(widgets::acceptsCharacter(UITextFilter::PRINTABLE, U' '));
    REQUIRE_FALSE(widgets::acceptsCharacter(UITextFilter::PRINTABLE, U'\n'));
    REQUIRE_FALSE(widgets::acceptsCharacter(UITextFilter::PRINTABLE, 0x7F));
    REQUIRE_FALSE(widgets::acceptsCharacter(UITextFilter::PRINTABLE, 0xD800));
}

TEST_CASE("characters are encoded in UTF-8", "[ui][widgets][text]")
{
    REQUIRE(widgets::encodeUtf8(U'A') == "A");
    REQUIRE(widgets::encodeUtf8(U'\u00E9') == "\xC3\xA9");
    REQUIRE(widgets::encodeUtf8(U'\u20AC') == "\xE2\x82\xAC");
    REQUIRE(widgets::encodeUtf8(0x1F680) == "\xF0\x9F\x9A\x80");
    REQUIRE(widgets::encodeUtf8(0x110000).empty());
    REQUIRE(widgets::characterCount("h\xC3\xA9llo") == 5);
    REQUIRE(widgets::characterIndex("h\xC3\xA9llo", 3) == 2);
}

TEST_CASE("typing inserts at the caret", "[ui][widgets][text]")
{
    UITextInput input = inputWith("ac", 1);

    REQUIRE(widgets::insertCharacter(input, U'b'));
    REQUIRE(input.text == "abc");
    REQUIRE(input.caret == 2);
    REQUIRE(widgets::insertCharacter(input, U'\u00E9'));
    REQUIRE(input.text == "ab\xC3\xA9"
                          "c");
    REQUIRE(input.caret == 4);
}

TEST_CASE("the filter and the max length are enforced", "[ui][widgets][text]")
{
    UITextInput input;
    input.filter = UITextFilter::DIGITS;
    input.maxLength = 4;

    REQUIRE_FALSE(widgets::insertCharacter(input, U'x'));
    for (const char32_t digit : {U'4', U'2', U'4', U'2', U'9'})
        static_cast<void>(widgets::insertCharacter(input, digit));
    REQUIRE(input.text == "4242");
}

TEST_CASE("max length counts characters, not bytes", "[ui][widgets][text]")
{
    UITextInput input;
    input.maxLength = 2;

    REQUIRE(widgets::insertCharacter(input, U'\u00E9'));
    REQUIRE(widgets::insertCharacter(input, U'\u00E9'));
    REQUIRE_FALSE(widgets::insertCharacter(input, U'a'));
    REQUIRE(widgets::characterCount(input.text) == 2);
}

TEST_CASE("erasing and moving never split a character", "[ui][widgets][text]")
{
    UITextInput input = inputWith("a\xC3\xA9"
                                  "b",
                                  3);

    widgets::moveCaret(input, false);
    REQUIRE(input.caret == 1);
    widgets::moveCaret(input, true);
    REQUIRE(input.caret == 3);
    REQUIRE(widgets::eraseBeforeCaret(input));
    REQUIRE(input.text == "ab");
    REQUIRE(input.caret == 1);
    REQUIRE(widgets::eraseAfterCaret(input));
    REQUIRE(input.text == "a");
    REQUIRE_FALSE(widgets::eraseAfterCaret(input));
}

TEST_CASE("a caret out of range is repaired", "[ui][widgets][text]")
{
    UITextInput input = inputWith("\xC3\xA9", 1);

    REQUIRE(widgets::eraseBeforeCaret(input) == false);
    input.caret = 99;
    REQUIRE(widgets::eraseBeforeCaret(input));
    REQUIRE(input.text.empty());
}

TEST_CASE("slider values are clamped and snapped", "[ui][widgets][slider]")
{
    UISlider slider;
    slider.min = 0.0f;
    slider.max = 100.0f;
    slider.step = 10.0f;

    REQUIRE(widgets::snapValue(slider, 43.0f) == 40.0f);
    REQUIRE(widgets::snapValue(slider, 47.0f) == 50.0f);
    REQUIRE(widgets::snapValue(slider, -5.0f) == 0.0f);
    REQUIRE(widgets::snapValue(slider, 500.0f) == 100.0f);
    slider.step = 0.0f;
    REQUIRE(widgets::snapValue(slider, 43.5f) == 43.5f);
}

TEST_CASE("slider value follows the pointer", "[ui][widgets][slider]")
{
    UISlider slider;
    slider.min = 0.0f;
    slider.max = 100.0f;
    slider.step = 1.0f;
    const Rect bounds{100.0f, 0.0f, 200.0f, 20.0f};

    REQUIRE(widgets::sliderValueAt(slider, bounds, 200.0f) == 50.0f);
    REQUIRE(widgets::sliderValueAt(slider, bounds, 0.0f) == 0.0f);
    REQUIRE(widgets::sliderValueAt(slider, bounds, 999.0f) == 100.0f);
}

TEST_CASE("slider steps and reports changes", "[ui][widgets][slider]")
{
    UISlider slider;
    slider.step = 0.25f;

    REQUIRE(widgets::stepSlider(slider, true));
    REQUIRE(slider.value == 0.25f);
    REQUIRE(widgets::sliderRatio(slider) == 0.25f);
    REQUIRE(widgets::stepSlider(slider, false));
    REQUIRE_FALSE(widgets::stepSlider(slider, false));
    REQUIRE(slider.value == 0.0f);
}

TEST_CASE("dropdown options sit below the box", "[ui][widgets][dropdown]")
{
    UIDropdown dropdown;
    dropdown.options = {"720p", "1080p", "native"};
    const Rect bounds{10.0f, 20.0f, 100.0f, 30.0f};

    REQUIRE(widgets::dropdownOptionBounds(bounds, 1) ==
            Rect{10.0f, 80.0f, 100.0f, 30.0f});
    REQUIRE_FALSE(widgets::dropdownOptionAt(dropdown, bounds, 50.0f, 85.0f));
    dropdown.open = true;
    REQUIRE(widgets::dropdownOptionAt(dropdown, bounds, 50.0f, 85.0f) == 1u);
    REQUIRE_FALSE(widgets::dropdownOptionAt(dropdown, bounds, 50.0f, 35.0f));
    REQUIRE_FALSE(widgets::dropdownOptionAt(dropdown, bounds, 50.0f, 145.0f));
    REQUIRE_FALSE(widgets::dropdownOptionAt(dropdown, bounds, 200.0f, 85.0f));
}

TEST_CASE("scroll list rows map to the bounds", "[ui][widgets][list]")
{
    UIScrollList list = listOf(10, 4);
    const Rect bounds{0.0f, 0.0f, 100.0f, 80.0f};

    REQUIRE(widgets::scrollRowBounds(list, bounds, 2) ==
            Rect{0.0f, 40.0f, 100.0f, 20.0f});
    REQUIRE(widgets::scrollRowAt(list, bounds, 10.0f, 45.0f) == 2u);
    list.scrollOffset = 3;
    REQUIRE(widgets::scrollRowAt(list, bounds, 10.0f, 45.0f) == 5u);
    REQUIRE_FALSE(widgets::scrollRowAt(list, bounds, 10.0f, 200.0f));
    REQUIRE_FALSE(widgets::scrollRowAt(listOf(1, 4), bounds, 10.0f, 45.0f));
}

TEST_CASE("scrolling stays inside the list", "[ui][widgets][list]")
{
    UIScrollList list = listOf(10, 4);

    REQUIRE_FALSE(widgets::scrollBy(list, -1));
    REQUIRE(widgets::scrollBy(list, 3));
    REQUIRE(list.scrollOffset == 3);
    REQUIRE(widgets::scrollBy(list, 100));
    REQUIRE(list.scrollOffset == 6);
    widgets::ensureVisible(list, 1);
    REQUIRE(list.scrollOffset == 1);
    widgets::ensureVisible(list, 9);
    REQUIRE(list.scrollOffset == 6);
    REQUIRE(widgets::visibleRowCount(listOf(3, 0)) == 1);
}
