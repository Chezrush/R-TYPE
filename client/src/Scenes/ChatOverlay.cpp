/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ChatOverlay.cpp
*/

#include "Client/Scenes/ChatOverlay.hpp"

#include <cstddef>
#include <utility>

#include "Client/Events/LobbyEvents.hpp"
#include "Client/Events/OverlayEvents.hpp"
#include "Engine/Core/SceneRouter.hpp"
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
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UITextInput;
using engine::ecs::components::UITextSubmittedEvent;

constexpr const char *INPUT_ACTION = "chat.input";
constexpr Vec2 LOG_OFFSET{16.0f, -100.0f};
constexpr Vec2 LOG_SIZE{600.0f, 132.0f};
constexpr std::size_t VISIBLE_LINES = 6;
constexpr Vec2 INPUT_OFFSET{16.0f, -60.0f};
constexpr Vec2 INPUT_SIZE{600.0f, 32.0f};
constexpr unsigned int TEXT_SIZE = 16;

} // namespace

ChatOverlay::ChatOverlay(engine::ecs::UISceneContext context,
                         lobby::ChatService &chat)
    : UIOverlay(std::move(context)),
      _chat(chat)
{}

void ChatOverlay::onExit()
{
    leave();
    UIOverlay::onExit();
}

void ChatOverlay::onPause()
{
    leave();
    UIOverlay::onPause();
}

std::optional<Entity> ChatOverlay::inputField() const noexcept
{
    return _input;
}

std::optional<Entity> ChatOverlay::logList() const noexcept
{
    return _log;
}

void ChatOverlay::activate()
{
    buildWidgets();
    refreshLog();
    listen<UITextSubmittedEvent>([this](const UITextSubmittedEvent &event) {
        if (event.action != INPUT_ACTION)
            return;
        _chat.send(event.text);
        bus().publish(engine::core::OverlayCloseRequested{});
    });
    listen<UITextKeyEvent>([this](const UITextKeyEvent &event) {
        if (event.action == INPUT_ACTION && event.key == UIKey::ESCAPE)
            bus().publish(engine::core::OverlayCloseRequested{});
    });
    listen<events::ChatMessageReceived>(
        [this](const events::ChatMessageReceived &) { refreshLog(); });
    bus().publish(events::TextEntryStarted{});
}

void ChatOverlay::buildWidgets()
{
    const Entity log = spawn(UIAnchor::BOTTOM_LEFT, LOG_OFFSET, LOG_SIZE);
    const Entity input = spawn(UIAnchor::BOTTOM_LEFT, INPUT_OFFSET, INPUT_SIZE);
    UIScrollList list;
    UITextInput field;

    list.visibleRows = VISIBLE_LINES;
    list.style.fontId = fontId();
    list.style.characterSize = TEXT_SIZE;
    field.placeholder = "Say something, Enter sends";
    field.maxLength = lobby::MAX_CHAT_LENGTH;
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

void ChatOverlay::refreshLog()
{
    if (!_log.has_value())
        return;
    auto &list = widget<UIScrollList>(*_log);

    list.rows = _chat.model().lines();
    list.scrollOffset =
        list.rows.size() > VISIBLE_LINES ? list.rows.size() - VISIBLE_LINES : 0;
}

void ChatOverlay::leave()
{
    if (isActive())
        bus().publish(events::TextEntryEnded{});
}

} // namespace client::scenes
