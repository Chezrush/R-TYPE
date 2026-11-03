/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RoomBrowserScene.cpp
*/

#include "Client/Scenes/RoomBrowserScene.hpp"

#include <cstddef>
#include <utility>

#include "Client/Events/OverlayEvents.hpp"
#include "Client/Lobby/LobbyText.hpp"
#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UIDropdown.hpp"
#include "Engine/Ecs/Components/UIScrollList.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITextInput.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::UIColorRole;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UIDropdown;
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextInput;

constexpr const char *REFRESH_ACTION = "rooms.refresh";
constexpr const char *JOIN_ACTION = "rooms.join";
constexpr const char *BACK_ACTION = "rooms.back";
constexpr const char *CREATE_ACTION = "rooms.create";
constexpr unsigned int TITLE_TEXT_SIZE = 40;
constexpr unsigned int TEXT_SIZE = 18;
constexpr Vec2 TITLE_OFFSET{0.0f, -215.0f};
constexpr Vec2 TITLE_SIZE{600.0f, 44.0f};
constexpr Vec2 LIST_OFFSET{-90.0f, -60.0f};
constexpr Vec2 LIST_SIZE{560.0f, 260.0f};
constexpr std::size_t VISIBLE_ROOMS = 8;
constexpr float SIDE_X = 290.0f;
constexpr Vec2 SIDE_BUTTON_SIZE{180.0f, 46.0f};
constexpr float FORM_Y = 120.0f;
constexpr Vec2 FORM_LABEL_OFFSET{-90.0f, 86.0f};
constexpr Vec2 FORM_LABEL_SIZE{560.0f, 26.0f};
constexpr Vec2 NAME_OFFSET{-150.0f, FORM_Y};
constexpr Vec2 NAME_SIZE{440.0f, 36.0f};
constexpr Vec2 PLAYERS_OFFSET{120.0f, FORM_Y};
constexpr Vec2 PLAYERS_SIZE{80.0f, 36.0f};
constexpr Vec2 STATUS_OFFSET{0.0f, 180.0f};
constexpr Vec2 STATUS_SIZE{800.0f, 30.0f};
constexpr std::uint8_t SMALLEST_ROOM = 2;

} // namespace

RoomBrowserScene::RoomBrowserScene(engine::ecs::UISceneContext context)
    : UIScene(std::move(context))
{}

Entity RoomBrowserScene::part(BrowserWidget which) const
{
    return _parts.at(static_cast<std::size_t>(which));
}

bool RoomBrowserScene::isWaiting() const noexcept
{
    return _waiting;
}

void RoomBrowserScene::activate()
{
    _parts.clear();
    _waiting = false;
    addLabel("Rooms", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE);
    buildList();
    buildForm();
    buildActions();
    listenToWidgets();
    listenToServer();
    bus().publish(events::RoomListRequested{});
}

void RoomBrowserScene::buildList()
{
    const Entity list = spawn(UIAnchor::CENTER, LIST_OFFSET, LIST_SIZE);
    UIScrollList rooms;

    rooms.visibleRows = VISIBLE_ROOMS;
    rooms.isFocused = true;
    rooms.style.fontId = fontId();
    rooms.style.characterSize = TEXT_SIZE;
    registry().addComponent(list, std::move(rooms));
    _parts.push_back(list);
    float y = LIST_OFFSET.y - LIST_SIZE.y / 2.0f + SIDE_BUTTON_SIZE.y / 2.0f;
    for (const auto &[label, action] :
         {std::pair{"Refresh", REFRESH_ACTION}, std::pair{"Join", JOIN_ACTION},
          std::pair{"Back", BACK_ACTION}}) {
        _parts.push_back(
            addButton(label, action, {SIDE_X, y}, SIDE_BUTTON_SIZE));
        y += SIDE_BUTTON_SIZE.y + 12.0f;
    }
}

void RoomBrowserScene::buildForm()
{
    const Entity name = spawn(UIAnchor::CENTER, NAME_OFFSET, NAME_SIZE);
    const Entity players =
        spawn(UIAnchor::CENTER, PLAYERS_OFFSET, PLAYERS_SIZE);
    UITextInput input;
    UIDropdown sizes;

    addLabel("New room: name and players", FORM_LABEL_OFFSET, FORM_LABEL_SIZE,
             TEXT_SIZE);
    input.placeholder = "Room name";
    input.maxLength = lobby::MAX_ROOM_NAME;
    input.style.fontId = fontId();
    sizes.options = {"2", "3", "4"};
    sizes.selected = 2;
    sizes.style.fontId = fontId();
    registry().addComponent(name, std::move(input));
    registry().addComponent(players, std::move(sizes));
    _parts.push_back(name);
    _parts.push_back(players);
}

void RoomBrowserScene::buildActions()
{
    _parts.push_back(
        addButton("Create", CREATE_ACTION, {SIDE_X, FORM_Y}, SIDE_BUTTON_SIZE));
    _parts.push_back(addLabel("", STATUS_OFFSET, STATUS_SIZE, TEXT_SIZE));
}

void RoomBrowserScene::listenToWidgets()
{
    listen<UIButtonClickedEvent>(
        [this](const UIButtonClickedEvent &event) { onButton(event); });
    listen<engine::ecs::components::UITextSubmittedEvent>(
        [this](const auto &) { create(); });
}

void RoomBrowserScene::listenToServer()
{
    listen<events::RoomListReceived>(
        [this](const events::RoomListReceived &list) { onRooms(list); });
    listen<events::JoinFailed>([this](const events::JoinFailed &failure) {
        _waiting = false;
        showStatus(lobby::joinFailureMessage(failure.reason), true);
    });
    listen<events::LobbyUpdated>([this](const events::LobbyUpdated &) {
        if (_waiting)
            bus().publish(NavigationRequested{SceneRoute::LOBBY});
    });
}

void RoomBrowserScene::onButton(const UIButtonClickedEvent &event)
{
    if (event.action == REFRESH_ACTION) {
        bus().publish(events::RoomListRequested{});
    } else if (event.action == JOIN_ACTION) {
        join();
    } else if (event.action == CREATE_ACTION) {
        create();
    } else if (event.action == BACK_ACTION) {
        bus().publish(events::DisconnectRequested{});
        bus().publish(NavigationRequested{SceneRoute::MAIN_MENU});
    }
}

void RoomBrowserScene::onRooms(const events::RoomListReceived &list)
{
    auto &rows = widget<UIScrollList>(part(BrowserWidget::LIST));

    _rooms = list.rooms;
    rows.rows.clear();
    for (const events::RoomInfo &room : _rooms)
        rows.rows.push_back(lobby::roomRowText(room));
    if (rows.selected.has_value() && *rows.selected >= _rooms.size())
        rows.selected.reset();
    if (_rooms.empty())
        showStatus("No room yet: create one.", false);
}

void RoomBrowserScene::join()
{
    const auto selected =
        widget<UIScrollList>(part(BrowserWidget::LIST)).selected;

    if (_waiting)
        return;
    if (!selected.has_value() || *selected >= _rooms.size()) {
        showStatus("Select a room first.", true);
        return;
    }
    const events::RoomInfo &room = _rooms.at(*selected);
    if (lobby::isRoomFull(room)) {
        showStatus(lobby::joinFailureMessage(events::JoinFailure::ROOM_FULL),
                   true);
        return;
    }
    _waiting = true;
    showStatus("Joining " + room.name + "...", false);
    bus().publish(events::JoinRoomRequested{room.id});
}

void RoomBrowserScene::create()
{
    const std::string &name =
        widget<UITextInput>(part(BrowserWidget::NAME)).text;
    const std::size_t size =
        widget<UIDropdown>(part(BrowserWidget::MAX_PLAYERS)).selected;

    if (_waiting)
        return;
    if (!lobby::isValidRoomName(name)) {
        showStatus("Room names have 1 to 24 plain characters.", true);
        return;
    }
    _waiting = true;
    showStatus("Creating " + name + "...", false);
    bus().publish(events::CreateRoomRequested{
        name, static_cast<std::uint8_t>(SMALLEST_ROOM + size)});
}

void RoomBrowserScene::showStatus(const std::string &text, bool error)
{
    auto &status = widget<UIText>(part(BrowserWidget::STATUS));

    status.text = text;
    status.colorRole = error ? UIColorRole::ERROR_TEXT : UIColorRole::TEXT;
    status.color = error ? engine::ecs::components::UIColor{255, 110, 100, 255}
                         : engine::ecs::components::UIColor{};
}

} // namespace client::scenes
