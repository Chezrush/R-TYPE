/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConnectScene.cpp
*/

#include "Client/Scenes/ConnectScene.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>

#include "Client/Events/GameEvents.hpp"
#include "Client/Scenes/SceneRoute.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UIButtonState;
using engine::ecs::components::UICancelEvent;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextChangedEvent;
using engine::ecs::components::UITextFilter;
using engine::ecs::components::UITextInput;
using engine::ecs::components::UITextSubmittedEvent;

constexpr const char *HOST_ACTION = "connect.host";
constexpr const char *PORT_ACTION = "connect.port";
constexpr const char *NICKNAME_ACTION = "connect.nickname";
constexpr const char *CONNECT_ACTION = "connect.connect";
constexpr const char *BACK_ACTION = "connect.back";

constexpr std::size_t HOST_PART = 0;
constexpr std::size_t PORT_PART = 1;
constexpr std::size_t NICKNAME_PART = 2;
constexpr std::size_t CONNECT_PART = 3;
constexpr std::size_t BACK_PART = 4;
constexpr std::size_t STATUS_PART = 5;

constexpr std::size_t MAX_HOST_CHARACTERS = 253;
constexpr std::size_t MAX_PORT_CHARACTERS = 5;
constexpr unsigned int TITLE_TEXT_SIZE = 48;
constexpr unsigned int LABEL_TEXT_SIZE = 18;
constexpr unsigned int STATUS_TEXT_SIZE = 20;
constexpr Vec2 TITLE_OFFSET{0.0f, -205.0f};
constexpr Vec2 TITLE_SIZE{600.0f, 60.0f};
constexpr Vec2 FIELD_SIZE{440.0f, 44.0f};
constexpr Vec2 LABEL_SIZE{440.0f, 24.0f};
constexpr float LABEL_GAP = 34.0f;
constexpr float HOST_Y = -120.0f;
constexpr float PORT_Y = -45.0f;
constexpr float NICKNAME_Y = 30.0f;
constexpr Vec2 BUTTON_SIZE{210.0f, 52.0f};
constexpr float BUTTON_X = 115.0f;
constexpr float BUTTON_Y = 95.0f;
constexpr Vec2 STATUS_OFFSET{0.0f, 160.0f};
constexpr Vec2 STATUS_SIZE{800.0f, 32.0f};
constexpr UIColor ERROR_COLOR{255, 110, 100, 255};
constexpr UIColor PROGRESS_COLOR{235, 235, 255, 255};

UITextInput makeInput(std::string text, const char *action,
                      std::size_t maxLength, UITextFilter filter)
{
    UITextInput input;

    input.caret = text.size();
    input.text = std::move(text);
    input.action = action;
    input.maxLength = maxLength;
    input.filter = filter;
    return input;
}

} // namespace

ConnectScene::ConnectScene(engine::ecs::UISceneContext context,
                           ConnectForm form)
    : UIScene(std::move(context)),
      _form(std::move(form))
{}

const ConnectForm &ConnectScene::form() const noexcept
{
    return _form;
}

bool ConnectScene::isConnecting() const noexcept
{
    return _connecting;
}

const std::string &ConnectScene::message() const noexcept
{
    return _message;
}

Entity ConnectScene::hostField() const
{
    return part(HOST_PART);
}

Entity ConnectScene::portField() const
{
    return part(PORT_PART);
}

Entity ConnectScene::nicknameField() const
{
    return part(NICKNAME_PART);
}

Entity ConnectScene::connectButton() const
{
    return part(CONNECT_PART);
}

Entity ConnectScene::backButton() const
{
    return part(BACK_PART);
}

Entity ConnectScene::statusLabel() const
{
    return part(STATUS_PART);
}

Entity ConnectScene::part(std::size_t index) const
{
    return _parts.at(index);
}

void ConnectScene::activate()
{
    _parts.clear();
    addLabel("Join a server", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE);
    buildFields();
    buildActions();
    listenToWidgets();
    listenToNetwork();
    setEditable(!_connecting);
    setStatus(_message, _connecting ? PROGRESS_COLOR : ERROR_COLOR);
    focusOn(_connecting ? backButton() : hostField());
}

Entity ConnectScene::addField(const std::string &title, float y,
                              UITextInput input)
{
    const Entity entity = spawn(UIAnchor::CENTER, {0.0f, y}, FIELD_SIZE);

    addLabel(title, {0.0f, y - LABEL_GAP}, LABEL_SIZE, LABEL_TEXT_SIZE);
    input.style.fontId = fontId();
    registry().addComponent(entity, std::move(input));
    return entity;
}

void ConnectScene::buildFields()
{
    _parts.push_back(
        addField("Address", HOST_Y,
                 makeInput(_form.host, HOST_ACTION, MAX_HOST_CHARACTERS,
                           UITextFilter::PRINTABLE)));
    _parts.push_back(
        addField("Port", PORT_Y,
                 makeInput(_form.port, PORT_ACTION, MAX_PORT_CHARACTERS,
                           UITextFilter::DIGITS)));
    _parts.push_back(
        addField("Nickname", NICKNAME_Y,
                 makeInput(_form.nickname, NICKNAME_ACTION, MAX_NICKNAME_BYTES,
                           UITextFilter::PRINTABLE)));
}

void ConnectScene::buildActions()
{
    _parts.push_back(addButton("Connect", CONNECT_ACTION, {-BUTTON_X, BUTTON_Y},
                               BUTTON_SIZE));
    _parts.push_back(
        addButton("Back", BACK_ACTION, {BUTTON_X, BUTTON_Y}, BUTTON_SIZE));
    _parts.push_back(
        addLabel("", STATUS_OFFSET, STATUS_SIZE, STATUS_TEXT_SIZE));
}

void ConnectScene::listenToWidgets()
{
    listen<UITextChangedEvent>(
        [this](const UITextChangedEvent &event) { onText(event); });
    listen<UITextSubmittedEvent>(
        [this](const UITextSubmittedEvent &) { submit(); });
    listen<UICancelEvent>([this](const UICancelEvent &) { goBack(); });
    listen<UIButtonClickedEvent>([this](const UIButtonClickedEvent &event) {
        if (event.action == CONNECT_ACTION)
            submit();
        else if (event.action == BACK_ACTION)
            goBack();
    });
}

void ConnectScene::listenToNetwork()
{
    listen<events::ConnectionStarted>(
        [this](const events::ConnectionStarted &event) {
            if (_connecting)
                showProgress(connectingMessage(event.host, event.port));
        });
    listen<events::ConnectionFailed>(
        [this](const events::ConnectionFailed &event) {
            if (_connecting)
                showError(failureMessage(event.reason));
        });
    listen<events::WelcomeReceived>([this](const events::WelcomeReceived &) {
        if (_connecting)
            bus().publish(NavigationRequested{SceneRoute::GAME});
    });
}

void ConnectScene::onText(const UITextChangedEvent &event)
{
    if (event.action == HOST_ACTION)
        _form.host = event.text;
    else if (event.action == PORT_ACTION)
        _form.port = event.text;
    else if (event.action == NICKNAME_ACTION)
        _form.nickname = event.text;
}

void ConnectScene::submit()
{
    if (_connecting)
        return;
    if (const auto error = validationError(_form)) {
        showError(*error);
        return;
    }
    const std::uint16_t port = parsePort(_form.port).value_or(0);

    showProgress(connectingMessage(_form.host, port));
    bus().publish(events::ConnectRequested{_form.host, port, _form.nickname});
}

void ConnectScene::goBack()
{
    if (_connecting)
        bus().publish(events::ConnectCancelled{});
    bus().publish(NavigationRequested{SceneRoute::MAIN_MENU});
}

void ConnectScene::showError(const std::string &text)
{
    const bool wasConnecting = _connecting;

    _connecting = false;
    setEditable(true);
    setStatus(text, ERROR_COLOR);
    if (wasConnecting)
        focusOn(hostField());
}

void ConnectScene::showProgress(const std::string &text)
{
    _connecting = true;
    setEditable(false);
    setStatus(text, PROGRESS_COLOR);
    focusOn(backButton());
}

void ConnectScene::setStatus(const std::string &text, UIColor color)
{
    auto &status = widget<UIText>(statusLabel());

    _message = text;
    status.text = text;
    status.color = color;
    status.colorRole = color == ERROR_COLOR
                           ? engine::ecs::UIColorRole::ERROR_TEXT
                           : engine::ecs::UIColorRole::TEXT;
}

void ConnectScene::setEditable(bool editable)
{
    for (const Entity field : {hostField(), portField(), nicknameField()})
        widget<UITextInput>(field).enabled = editable;
    widget<UIButton>(connectButton()).state =
        editable ? UIButtonState::NORMAL : UIButtonState::DISABLED;
}

void ConnectScene::focusOn(Entity target)
{
    for (const Entity field : {hostField(), portField(), nicknameField()})
        widget<UITextInput>(field).isFocused = field == target;
    for (const Entity button : {connectButton(), backButton()})
        widget<UIButton>(button).isFocused = button == target;
}

} // namespace client::scenes
