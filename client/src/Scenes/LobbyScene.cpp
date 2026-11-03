/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LobbyScene.cpp
*/

#include "Client/Scenes/LobbyScene.hpp"

#include <cstddef>
#include <memory>
#include <utility>

#include "Client/Events/GameEvents.hpp"
#include "Client/Hud/HudView.hpp"
#include "Client/Lobby/LobbyText.hpp"
#include "Client/Scenes/ChatOverlay.hpp"
#include "Client/Scenes/RoomBrowserScene.hpp"
#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIScrollList.hpp"
#include "Engine/Ecs/Components/UIShape.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITextInput.hpp"
#include "Engine/Ecs/Components/UIToggle.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::UIColorRole;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UIButtonState;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UIShape;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextAlign;
using engine::ecs::components::UITextInput;
using engine::ecs::components::UITextSubmittedEvent;
using engine::ecs::components::UIToggle;
using engine::ecs::components::UIValueChangedEvent;
using hud::HudView;

constexpr const char *READY_ACTION = "lobby.ready";
constexpr const char *START_ACTION = "lobby.start";
constexpr const char *LEAVE_ACTION = "lobby.leave";
constexpr const char *CHAT_ACTION = "lobby.chat";
constexpr unsigned int TITLE_TEXT_SIZE = 36;
constexpr unsigned int TEXT_SIZE = 18;
constexpr Vec2 TITLE_OFFSET{0.0f, -215.0f};
constexpr Vec2 TITLE_SIZE{700.0f, 40.0f};
constexpr float FIRST_SLOT_Y = -160.0f;
constexpr float SLOT_SPACING = 44.0f;
constexpr Vec2 TAG_SIZE{50.0f, 34.0f};
constexpr float TAG_X = -330.0f;
constexpr Vec2 SHAPE_SIZE{16.0f, 16.0f};
constexpr float SHAPE_X = -290.0f;
constexpr Vec2 SLOT_TEXT_SIZE{520.0f, 34.0f};
constexpr float SLOT_TEXT_X = -10.0f;
constexpr float BUTTONS_Y = 30.0f;
constexpr Vec2 BUTTON_SIZE{200.0f, 46.0f};
constexpr Vec2 READY_SIZE{150.0f, 34.0f};
constexpr Vec2 LOG_OFFSET{0.0f, 125.0f};
constexpr Vec2 LOG_SIZE{760.0f, 110.0f};
constexpr std::size_t CHAT_ROWS = 5;
constexpr Vec2 INPUT_OFFSET{0.0f, 200.0f};
constexpr Vec2 INPUT_SIZE{760.0f, 30.0f};
constexpr UIColor FREE_COLOR{45, 45, 55, 255};

float slotY(std::size_t slot) noexcept
{
    return FIRST_SLOT_Y + SLOT_SPACING * static_cast<float>(slot);
}

} // namespace

LobbyScene::LobbyScene(engine::ecs::UISceneContext context,
                       lobby::ChatService &chat)
    : UIScene(std::move(context)),
      _chat(chat)
{}

Entity LobbyScene::part(LobbyWidget which) const
{
    return _parts.at(static_cast<std::size_t>(which));
}

Entity LobbyScene::slotTag(std::size_t slot) const
{
    return _tags.at(slot);
}

Entity LobbyScene::slotText(std::size_t slot) const
{
    return _texts.at(slot);
}

Entity LobbyScene::slotShape(std::size_t slot) const
{
    return _shapes.at(slot);
}

void LobbyScene::activate()
{
    _tags.clear();
    _texts.clear();
    _shapes.clear();
    _parts.clear();
    _parts.push_back(
        addLabel("Lobby", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE));
    buildSlots();
    buildButtons();
    buildChat();
    listenToWidgets();
    listenToServer();
    if (_lobby.has_value())
        onLobby(*_lobby);
    refreshChat();
}

void LobbyScene::buildSlots()
{
    for (std::size_t slot = 0; slot < events::LOBBY_SLOTS; ++slot)
        buildSlot(slot);
}

void LobbyScene::buildSlot(std::size_t slot)
{
    const Entity tag = addLabel(HudView::playerTag(slot), {TAG_X, slotY(slot)},
                                TAG_SIZE, TEXT_SIZE);
    const Entity shape =
        spawn(UIAnchor::CENTER, {SHAPE_X, slotY(slot)}, SHAPE_SIZE);
    const Entity text =
        addLabel("Free", {SLOT_TEXT_X, slotY(slot)}, SLOT_TEXT_SIZE, TEXT_SIZE);
    UIPanel panel;

    panel.color = FREE_COLOR;
    registry().addComponent(tag, std::move(panel));
    registry().addComponent(shape,
                            UIShape{HudView::playerShape(slot), FREE_COLOR,
                                    UIColorRole::TEXT_MUTED});
    widget<UIText>(text).align = UITextAlign::LEFT;
    _tags.push_back(tag);
    _shapes.push_back(shape);
    _texts.push_back(text);
    syncSlot(slot);
}

void LobbyScene::buildButtons()
{
    const Entity ready =
        spawn(UIAnchor::CENTER, {-250.0f, BUTTONS_Y}, READY_SIZE);
    UIToggle toggle;

    toggle.action = READY_ACTION;
    toggle.isFocused = true;
    registry().addComponent(ready, std::move(toggle));
    addLabel("Ready", {-245.0f, BUTTONS_Y}, {80.0f, 34.0f}, TEXT_SIZE);
    _parts.push_back(ready);
    _parts.push_back(
        addButton("Start", START_ACTION, {20.0f, BUTTONS_Y}, BUTTON_SIZE));
    _parts.push_back(
        addButton("Leave", LEAVE_ACTION, {250.0f, BUTTONS_Y}, BUTTON_SIZE));
    widget<UIButton>(part(LobbyWidget::START)).state = UIButtonState::DISABLED;
}

void LobbyScene::buildChat()
{
    const Entity log = spawn(UIAnchor::CENTER, LOG_OFFSET, LOG_SIZE);
    const Entity input = spawn(UIAnchor::CENTER, INPUT_OFFSET, INPUT_SIZE);
    UIScrollList list;
    UITextInput field;

    list.visibleRows = CHAT_ROWS;
    list.style.fontId = fontId();
    list.style.characterSize = TEXT_SIZE - 2;
    field.placeholder = "Chat: Enter sends, /mute 2 hides player 2";
    field.maxLength = lobby::MAX_CHAT_LENGTH;
    field.action = CHAT_ACTION;
    field.style.fontId = fontId();
    field.style.characterSize = TEXT_SIZE - 2;
    registry().addComponent(log, std::move(list));
    registry().addComponent(input, std::move(field));
    _parts.push_back(log);
    _parts.push_back(input);
}

void LobbyScene::listenToWidgets()
{
    listen<UIButtonClickedEvent>(
        [this](const UIButtonClickedEvent &event) { onButton(event); });
    listen<UIValueChangedEvent>([this](const UIValueChangedEvent &event) {
        if (event.action == READY_ACTION)
            bus().publish(events::ReadyChanged{event.value > 0.5f});
    });
    listen<UITextSubmittedEvent>([this](const UITextSubmittedEvent &event) {
        if (event.action != CHAT_ACTION)
            return;
        _chat.send(event.text);
        auto &field = widget<UITextInput>(part(LobbyWidget::CHAT_INPUT));
        field.text.clear();
        field.caret = 0;
        refreshChat();
    });
}

void LobbyScene::listenToServer()
{
    listen<events::LobbyUpdated>(
        [this](const events::LobbyUpdated &lobby) { onLobby(lobby); });
    listen<events::ChatMessageReceived>(
        [this](const events::ChatMessageReceived &) { refreshChat(); });
    listen<events::GameStateChanged>(
        [this](const events::GameStateChanged &state) {
            if (state.phase == events::GamePhase::PLAYING)
                bus().publish(NavigationRequested{SceneRoute::GAME});
        });
}

void LobbyScene::onButton(const UIButtonClickedEvent &event)
{
    if (event.action == START_ACTION) {
        bus().publish(events::StartGameRequested{});
    } else if (event.action == LEAVE_ACTION) {
        bus().publish(events::LeaveRoomRequested{});
        bus().publish(NavigationRequested{SceneRoute::ROOMS});
    }
}

void LobbyScene::onLobby(const events::LobbyUpdated &lobby)
{
    _lobby = lobby;
    widget<UIText>(part(LobbyWidget::TITLE)).text = lobby.roomName;
    for (std::size_t slot = 0; slot < events::LOBBY_SLOTS; ++slot)
        syncSlot(slot);
    if (lobby.localPlayer < events::LOBBY_SLOTS)
        widget<UIToggle>(part(LobbyWidget::READY)).checked =
            lobby.slots.at(lobby.localPlayer).ready;
    widget<UIButton>(part(LobbyWidget::START)).state =
        lobby::canStart(lobby) ? UIButtonState::NORMAL
                               : UIButtonState::DISABLED;
}

void LobbyScene::syncSlot(std::size_t slot)
{
    const bool occupied = _lobby.has_value() && _lobby->slots.at(slot).occupied;
    auto &tag = widget<UIPanel>(_tags.at(slot));
    auto &shape = widget<UIShape>(_shapes.at(slot));
    auto &label = widget<UIText>(_tags.at(slot));

    tag.color = occupied ? HudView::playerColor(slot) : FREE_COLOR;
    tag.colorRole = occupied ? HudView::playerRole(slot) : UIColorRole::NONE;
    shape.color = tag.color;
    shape.colorRole =
        occupied ? HudView::playerRole(slot) : UIColorRole::TEXT_MUTED;
    label.colorRole =
        occupied ? UIColorRole::TEXT_ON_PLAYER : UIColorRole::TEXT_MUTED;
    widget<UIText>(_texts.at(slot)).text =
        _lobby.has_value() ? lobby::slotText(*_lobby, slot) : "Free";
}

void LobbyScene::refreshChat()
{
    auto &list = widget<UIScrollList>(part(LobbyWidget::CHAT_LOG));

    list.rows = _chat.model().lines();
    list.scrollOffset =
        list.rows.size() > CHAT_ROWS ? list.rows.size() - CHAT_ROWS : 0;
}

void addLobbyRoutes(SceneRouter &router,
                    const engine::ecs::UISceneContext &context,
                    lobby::ChatService &chat)
{
    router.setRoute(SceneRoute::ROOMS, [context] {
        return std::make_unique<RoomBrowserScene>(context);
    });
    router.setRoute(SceneRoute::LOBBY, [context, &chat] {
        return std::make_unique<LobbyScene>(context, chat);
    });
    router.setRoute(SceneRoute::CHAT, [context, &chat] {
        return std::make_unique<ChatOverlay>(context, chat);
    });
}

} // namespace client::scenes
