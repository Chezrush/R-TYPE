/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIWidgets.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"

using engine::core::EventBus;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::components::Rect;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UIDropdown;
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UISelectionChangedEvent;
using engine::ecs::components::UISlider;
using engine::ecs::components::UITextChangedEvent;
using engine::ecs::components::UITextFilter;
using engine::ecs::components::UITextInput;
using engine::ecs::components::UIToggle;
using engine::ecs::components::UITransform;
using engine::ecs::components::UIValueChangedEvent;
using engine::ecs::systems::UIInteractionSystem;
using engine::ecs::systems::UIKey;

namespace {

class WidgetScreen {
public:
    WidgetScreen()
    {
        engine::ecs::registerUIComponents(_registry);
        _bus.subscribe<UITextChangedEvent>(
            [this](const UITextChangedEvent &event) {
                texts.push_back(event);
            });
        _bus.subscribe<UIValueChangedEvent>(
            [this](const UIValueChangedEvent &event) {
                values.push_back(event);
            });
        _bus.subscribe<UISelectionChangedEvent>(
            [this](const UISelectionChangedEvent &event) {
                selections.push_back(event);
            });
        _bus.subscribe<UIButtonClickedEvent>(
            [this](const UIButtonClickedEvent &event) {
                clicks.push_back(event);
            });
    }

    template <typename Widget>
    Entity add(Rect bounds, Widget widget, int zIndex = 0)
    {
        const Entity entity = _registry.spawnEntity();
        UITransform transform;

        transform.actualBounds = bounds;
        transform.zIndex = zIndex;
        _registry.addComponent(entity, transform);
        _registry.addComponent(entity, std::move(widget));
        return entity;
    }

    template <typename Widget>
    Widget &get(Entity entity)
    {
        return *_registry.getComponents<Widget>().get(entity);
    }

    void click(float x, float y)
    {
        _system.onMouseDown(x, y);
        _system.onMouseUp(x, y);
        _system.process(_registry);
    }

    void key(UIKey key)
    {
        _system.onKeyPressed(key);
        _system.process(_registry);
    }

    void type(const std::u32string &characters)
    {
        for (const char32_t character : characters)
            _system.onTextEntered(character);
        _system.process(_registry);
    }

    UIInteractionSystem &system()
    {
        return _system;
    }

    Registry &registry()
    {
        return _registry;
    }

    [[nodiscard]] std::optional<Entity> focused() const
    {
        return UIInteractionSystem::focusedEntity(_registry);
    }

    std::vector<UITextChangedEvent> texts;
    std::vector<UIValueChangedEvent> values;
    std::vector<UISelectionChangedEvent> selections;
    std::vector<UIButtonClickedEvent> clicks;

private:
    EventBus _bus;
    Registry _registry;
    UIInteractionSystem _system{_bus};
};

UITextInput textInput(std::string action, UITextFilter filter,
                      std::size_t maxLength)
{
    UITextInput input;

    input.action = std::move(action);
    input.filter = filter;
    input.maxLength = maxLength;
    return input;
}

UISlider volumeSlider()
{
    UISlider slider;

    slider.max = 100.0f;
    slider.step = 10.0f;
    slider.action = "volume";
    return slider;
}

UIDropdown resolutionDropdown()
{
    UIDropdown dropdown;

    dropdown.options = {"720p", "1080p", "native"};
    dropdown.action = "resolution";
    return dropdown;
}

UIScrollList roomList(std::size_t rows)
{
    UIScrollList list;

    for (std::size_t i = 0; i < rows; ++i)
        list.rows.push_back("room " + std::to_string(i));
    list.visibleRows = 4;
    list.action = "room";
    return list;
}

constexpr Rect FIELD{0.0f, 0.0f, 200.0f, 30.0f};
constexpr Rect SECOND{0.0f, 50.0f, 200.0f, 30.0f};
constexpr Rect LIST{0.0f, 100.0f, 200.0f, 80.0f};

} // namespace

TEST_CASE("typing goes to the focused text input", "[ecs][ui][widgets][text]")
{
    WidgetScreen ui;
    const Entity port =
        ui.add(FIELD, textInput("port", UITextFilter::DIGITS, 5));

    ui.type(U"42");
    REQUIRE(ui.get<UITextInput>(port).text.empty());

    ui.click(10.0f, 10.0f);
    ui.type(U"4a24299");

    REQUIRE(ui.get<UITextInput>(port).text == "42429");
    REQUIRE(ui.texts.size() == 5);
    REQUIRE(ui.texts.back() ==
            UITextChangedEvent{port.index(), "port", "42429"});
}

TEST_CASE("editing keys move the caret and erase", "[ecs][ui][widgets][text]")
{
    WidgetScreen ui;
    const Entity name =
        ui.add(FIELD, textInput("name", UITextFilter::PRINTABLE, 16));
    ui.click(10.0f, 10.0f);
    ui.type(U"abc");

    ui.key(UIKey::LEFT);
    ui.key(UIKey::BACKSPACE);
    REQUIRE(ui.get<UITextInput>(name).text == "ac");
    ui.key(UIKey::HOME);
    ui.key(UIKey::DEL);
    REQUIRE(ui.get<UITextInput>(name).text == "c");
    ui.key(UIKey::END);
    ui.type(U" é");
    REQUIRE(ui.get<UITextInput>(name).text == "c \xC3\xA9");
    REQUIRE(ui.focused() == name);
}

TEST_CASE("Tab cycles through every kind of widget",
          "[ecs][ui][widgets][focus]")
{
    WidgetScreen ui;
    const Entity button = ui.add(FIELD, UIButton{});
    const Entity input =
        ui.add(SECOND, textInput("name", UITextFilter::PRINTABLE, 8));
    const Entity slider = ui.add(SECOND, volumeSlider());
    UIToggle disabled;
    disabled.enabled = false;
    ui.add(SECOND, disabled);
    const Entity dropdown = ui.add(SECOND, resolutionDropdown());
    const Entity list = ui.add(LIST, roomList(3));

    for (const Entity expected :
         {button, input, slider, dropdown, list, button}) {
        ui.key(UIKey::TAB);
        REQUIRE(ui.focused() == expected);
    }
    ui.key(UIKey::BACK_TAB);
    REQUIRE(ui.focused() == list);
}

TEST_CASE("only one widget is focused at a time", "[ecs][ui][widgets][focus]")
{
    WidgetScreen ui;
    const Entity input =
        ui.add(FIELD, textInput("name", UITextFilter::PRINTABLE, 8));
    const Entity toggle = ui.add(SECOND, UIToggle{});

    ui.click(10.0f, 10.0f);
    ui.click(10.0f, 60.0f);

    REQUIRE_FALSE(ui.get<UITextInput>(input).isFocused);
    REQUIRE(ui.get<UIToggle>(toggle).isFocused);
}

TEST_CASE("sliders follow clicks, drags and arrows",
          "[ecs][ui][widgets][slider]")
{
    WidgetScreen ui;
    const Entity slider = ui.add(FIELD, volumeSlider());

    ui.system().onMouseDown(100.0f, 10.0f);
    ui.system().onMouseMove(150.0f, 10.0f);
    ui.system().onMouseUp(150.0f, 10.0f);
    ui.system().process(ui.registry());
    REQUIRE(ui.get<UISlider>(slider).value == 80.0f);

    ui.key(UIKey::RIGHT);
    ui.key(UIKey::RIGHT);
    ui.key(UIKey::RIGHT);
    REQUIRE(ui.get<UISlider>(slider).value == 100.0f);
    ui.key(UIKey::HOME);
    REQUIRE(ui.values.size() == 5);
    REQUIRE(ui.values.back() ==
            UIValueChangedEvent{slider.index(), "volume", 0.0f});
}

TEST_CASE("toggles flip on click and on Enter / Space",
          "[ecs][ui][widgets][toggle]")
{
    WidgetScreen ui;
    UIToggle fullscreen;
    fullscreen.action = "fullscreen";
    const Entity toggle = ui.add(FIELD, fullscreen);

    ui.click(10.0f, 10.0f);
    REQUIRE(ui.get<UIToggle>(toggle).checked);
    ui.key(UIKey::SPACE);
    ui.key(UIKey::ENTER);
    REQUIRE(ui.get<UIToggle>(toggle).checked);
    REQUIRE(ui.values.size() == 3);
    REQUIRE(ui.values.back().value == 1.0f);
}

TEST_CASE("a toggle released outside is not flipped",
          "[ecs][ui][widgets][toggle]")
{
    WidgetScreen ui;
    const Entity toggle = ui.add(FIELD, UIToggle{});

    ui.system().onMouseDown(10.0f, 10.0f);
    ui.system().onMouseUp(500.0f, 500.0f);
    ui.system().process(ui.registry());

    REQUIRE_FALSE(ui.get<UIToggle>(toggle).checked);
    REQUIRE(ui.values.empty());
}

TEST_CASE("dropdowns open on click and select an option",
          "[ecs][ui][widgets][dropdown]")
{
    WidgetScreen ui;
    const Entity dropdown = ui.add(FIELD, resolutionDropdown());
    const Entity below = ui.add(SECOND, UIButton{});

    ui.click(10.0f, 10.0f);
    REQUIRE(ui.get<UIDropdown>(dropdown).open);
    ui.click(10.0f, 70.0f);

    REQUIRE_FALSE(ui.get<UIDropdown>(dropdown).open);
    REQUIRE(ui.get<UIDropdown>(dropdown).selected == 1);
    REQUIRE(ui.selections == std::vector<UISelectionChangedEvent>{
                                 {dropdown.index(), "resolution", 1, "1080p"}});
    REQUIRE(ui.clicks.empty());
    REQUIRE_FALSE(ui.get<UIButton>(below).isFocused);
}

TEST_CASE("dropdowns work with the keyboard", "[ecs][ui][widgets][dropdown]")
{
    WidgetScreen ui;
    const Entity dropdown = ui.add(FIELD, resolutionDropdown());
    ui.key(UIKey::TAB);

    ui.key(UIKey::RIGHT);
    REQUIRE(ui.get<UIDropdown>(dropdown).selected == 1);
    ui.key(UIKey::ENTER);
    REQUIRE(ui.get<UIDropdown>(dropdown).open);
    ui.key(UIKey::DOWN);
    ui.key(UIKey::DOWN);
    ui.key(UIKey::ESCAPE);
    REQUIRE_FALSE(ui.get<UIDropdown>(dropdown).open);
    REQUIRE(ui.get<UIDropdown>(dropdown).selected == 1);
    ui.key(UIKey::SPACE);
    ui.key(UIKey::UP);
    ui.key(UIKey::SPACE);
    REQUIRE(ui.get<UIDropdown>(dropdown).selected == 0);
    REQUIRE(ui.selections.size() == 2);
}

TEST_CASE("clicking elsewhere or Tab closes a dropdown",
          "[ecs][ui][widgets][dropdown]")
{
    WidgetScreen ui;
    const Entity dropdown = ui.add(FIELD, resolutionDropdown());
    ui.add({300.0f, 0.0f, 50.0f, 30.0f}, UIButton{});

    ui.click(10.0f, 10.0f);
    ui.click(500.0f, 500.0f);
    REQUIRE_FALSE(ui.get<UIDropdown>(dropdown).open);

    ui.click(10.0f, 10.0f);
    ui.key(UIKey::TAB);
    REQUIRE_FALSE(ui.get<UIDropdown>(dropdown).open);
}

TEST_CASE("scroll lists select rows and scroll", "[ecs][ui][widgets][list]")
{
    WidgetScreen ui;
    const Entity list = ui.add(LIST, roomList(10));

    ui.click(10.0f, 125.0f);
    REQUIRE(ui.get<UIScrollList>(list).selected == 1u);
    ui.system().onMouseWheel(10.0f, 125.0f, -2.0f);
    ui.system().process(ui.registry());
    REQUIRE(ui.get<UIScrollList>(list).scrollOffset == 2);

    ui.key(UIKey::END);
    REQUIRE(ui.get<UIScrollList>(list).selected == 9u);
    REQUIRE(ui.get<UIScrollList>(list).scrollOffset == 6);
    ui.key(UIKey::UP);
    REQUIRE(ui.selections.back() ==
            UISelectionChangedEvent{list.index(), "room", 8, "room 8"});
}

TEST_CASE("the topmost widget gets the click", "[ecs][ui][widgets][focus]")
{
    WidgetScreen ui;
    const Entity under = ui.add(FIELD, UIToggle{}, 0);
    const Entity over = ui.add(FIELD, UIToggle{}, 5);

    ui.click(10.0f, 10.0f);

    REQUIRE(ui.get<UIToggle>(over).checked);
    REQUIRE_FALSE(ui.get<UIToggle>(under).checked);
}

TEST_CASE("Enter on a text input does not click buttons",
          "[ecs][ui][widgets][text]")
{
    WidgetScreen ui;
    ui.add(FIELD, textInput("name", UITextFilter::PRINTABLE, 8));
    ui.add(SECOND, UIButton{});
    ui.click(10.0f, 10.0f);

    ui.key(UIKey::ENTER);
    ui.key(UIKey::SPACE);

    REQUIRE(ui.clicks.empty());
}

TEST_CASE("a disabled widget ignores input", "[ecs][ui][widgets][focus]")
{
    WidgetScreen ui;
    UISlider slider = volumeSlider();
    slider.enabled = false;
    const Entity entity = ui.add(FIELD, slider);

    ui.click(150.0f, 10.0f);
    ui.key(UIKey::TAB);

    REQUIRE(ui.get<UISlider>(entity).value == 0.0f);
    REQUIRE_FALSE(ui.focused().has_value());
    REQUIRE(ui.values.empty());
}
