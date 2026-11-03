/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleOverlay.cpp
*/

#include "Client/Scenes/ConsoleOverlay.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include "Client/Events/ConsoleEvents.hpp"
#include "Client/Events/OverlayEvents.hpp"
#include "Client/Events/SettingsEvents.hpp"
#include "Engine/Ecs/Components/UIScrollList.hpp"
#include "Engine/Ecs/Components/UITextInput.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::UIKey;
using engine::ecs::UITextKeyEvent;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UITextInput;
using engine::ecs::components::UITextSubmittedEvent;

constexpr const char *INPUT_ACTION = "console.input";
constexpr UIColor BACKDROP_COLOR{0, 0, 0, 110};
constexpr Vec2 LOG_OFFSET{0.0f, 12.0f};
constexpr Vec2 LOG_SIZE{840.0f, 260.0f};
constexpr Vec2 INPUT_OFFSET{0.0f, 278.0f};
constexpr Vec2 INPUT_SIZE{840.0f, 32.0f};
constexpr std::size_t VISIBLE_LINES = 10;
constexpr std::size_t MAX_LINE_LENGTH = 200;
constexpr unsigned int TEXT_SIZE = 16;

} // namespace

ConsoleOverlay::ConsoleOverlay(engine::ecs::UISceneContext context,
                               console::ConsoleService &service)
    : UIOverlay(std::move(context)),
      _service(service)
{}

void ConsoleOverlay::onExit()
{
    leave();
    UIOverlay::onExit();
}

void ConsoleOverlay::onPause()
{
    leave();
    UIOverlay::onPause();
}

void ConsoleOverlay::handleInput()
{
    if (_discardInput) {
        interaction().discardInput();
        return;
    }
    UIOverlay::handleInput();
}

std::optional<Entity> ConsoleOverlay::inputField() const noexcept
{
    return _input;
}

std::optional<Entity> ConsoleOverlay::logList() const noexcept
{
    return _log;
}

void ConsoleOverlay::activate()
{
    _discardInput = false;
    _service.setOpen(true);
    addBackdrop(BACKDROP_COLOR);
    buildWidgets();
    refreshLog();
    listenToWidgets();
    bus().publish(events::TextEntryStarted{});
}

void ConsoleOverlay::buildWidgets()
{
    const Entity log = spawn(UIAnchor::TOP_CENTER, LOG_OFFSET, LOG_SIZE);
    const Entity input = spawn(UIAnchor::TOP_CENTER, INPUT_OFFSET, INPUT_SIZE);
    UIScrollList list;
    UITextInput field;

    list.visibleRows = VISIBLE_LINES;
    list.style.fontId = fontId();
    list.style.characterSize = TEXT_SIZE;
    field.placeholder = "Type a command, Tab completes";
    field.maxLength = MAX_LINE_LENGTH;
    field.action = INPUT_ACTION;
    field.claimsNavigationKeys = true;
    field.isFocused = true;
    field.style.fontId = fontId();
    field.style.characterSize = TEXT_SIZE;
    registry().addComponent(log, std::move(list));
    registry().addComponent(input, std::move(field));
    _log = log;
    _input = input;
}

void ConsoleOverlay::listenToWidgets()
{
    listen<UITextSubmittedEvent>([this](const UITextSubmittedEvent &event) {
        if (event.action == INPUT_ACTION)
            submit(event.text);
    });
    listen<UITextKeyEvent>([this](const UITextKeyEvent &event) {
        if (event.action == INPUT_ACTION)
            onKey(event.key);
    });
    listen<events::ConsoleOutput>(
        [this](const events::ConsoleOutput &) { refreshLog(); });
    listen<events::RawKeyPressed>([this](const events::RawKeyPressed &event) {
        if (event.key == settings::KEY_GRAVE)
            close();
    });
}

void ConsoleOverlay::onKey(UIKey key)
{
    if (!_input.has_value())
        return;
    auto &model = _service.model();

    if (key == UIKey::ESCAPE) {
        close();
    } else if (key == UIKey::TAB) {
        complete();
    } else if (key == UIKey::UP) {
        if (const auto line = model.previous(widget<UITextInput>(*_input).text))
            setLine(*line);
    } else if (key == UIKey::DOWN) {
        if (const auto line = model.next())
            setLine(*line);
    }
}

void ConsoleOverlay::submit(const std::string &line)
{
    const auto command = _service.model().submit(line);

    setLine("");
    if (!command.has_value())
        return;
    _service.model().addOutput("> " + *command, events::ConsoleLevel::INFO);
    refreshLog();
    bus().publish(events::ConsoleCommandSubmitted{*command});
}

void ConsoleOverlay::complete()
{
    if (!_input.has_value())
        return;
    const console::Completion completion =
        _service.model().complete(widget<UITextInput>(*_input).text);

    setLine(completion.text);
    if (completion.candidates.empty())
        return;
    std::string list;
    for (const std::string &name : completion.candidates)
        list += (list.empty() ? "" : "  ") + name;
    _service.model().addOutput(list, events::ConsoleLevel::INFO);
    refreshLog();
}

void ConsoleOverlay::setLine(const std::string &line)
{
    if (!_input.has_value())
        return;
    auto &field = widget<UITextInput>(*_input);

    field.text = line.substr(0, MAX_LINE_LENGTH);
    field.caret = field.text.size();
}

void ConsoleOverlay::refreshLog()
{
    if (!_log.has_value())
        return;
    auto &list = widget<UIScrollList>(*_log);
    const auto &lines = _service.model().lines();

    list.rows.assign(lines.begin(), lines.end());
    list.scrollOffset =
        list.rows.size() > VISIBLE_LINES ? list.rows.size() - VISIBLE_LINES : 0;
}

void ConsoleOverlay::close()
{
    _discardInput = true;
    bus().publish(engine::core::OverlayCloseRequested{});
}

void ConsoleOverlay::leave()
{
    if (!isActive())
        return;
    _service.setOpen(false);
    bus().publish(events::TextEntryEnded{});
}

void addConsoleRoute(SceneRouter &router,
                     const engine::ecs::UISceneContext &context,
                     console::ConsoleService &service)
{
    router.setRoute(SceneRoute::CONSOLE, [context, &service] {
        return std::make_unique<ConsoleOverlay>(context, service);
    });
}

} // namespace client::scenes
