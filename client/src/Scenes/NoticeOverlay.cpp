/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NoticeOverlay.cpp
*/

#include "Client/Scenes/NoticeOverlay.hpp"

#include <stdexcept>
#include <utility>

#include "Client/Scenes/SceneRoute.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UICancelEvent;

constexpr engine::ecs::components::UIColor BACKDROP_COLOR{0, 0, 0, 200};
constexpr unsigned int TITLE_TEXT_SIZE = 40;
constexpr unsigned int MESSAGE_TEXT_SIZE = 26;
constexpr Vec2 TITLE_OFFSET{0.0f, -90.0f};
constexpr Vec2 TITLE_SIZE{700.0f, 52.0f};
constexpr Vec2 MESSAGE_OFFSET{0.0f, -20.0f};
constexpr Vec2 MESSAGE_SIZE{900.0f, 40.0f};
constexpr Vec2 BUTTON_OFFSET{0.0f, 60.0f};
constexpr Vec2 BUTTON_SIZE{200.0f, 52.0f};

} // namespace

NoticeOverlay::NoticeOverlay(engine::ecs::UISceneContext context,
                             std::string message)
    : UIOverlay(std::move(context)),
      _message(std::move(message))
{}

const std::string &NoticeOverlay::message() const noexcept
{
    return _message;
}

engine::ecs::Entity NoticeOverlay::okButton() const
{
    if (!_ok)
        throw std::logic_error("the notice is not shown");
    return *_ok;
}

void NoticeOverlay::activate()
{
    addBackdrop(BACKDROP_COLOR);
    addLabel("Notice", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE);
    addLabel(_message, MESSAGE_OFFSET, MESSAGE_SIZE, MESSAGE_TEXT_SIZE);
    _ok = addButton("OK", "notice.ok", BUTTON_OFFSET, BUTTON_SIZE);
    widget<UIButton>(*_ok).isFocused = true;
    listen<UIButtonClickedEvent>([this](const UIButtonClickedEvent &) {
        bus().publish(engine::core::OverlayCloseRequested{});
    });
    listen<UICancelEvent>([this](const UICancelEvent &) {
        bus().publish(engine::core::OverlayCloseRequested{});
    });
}

} // namespace client::scenes
