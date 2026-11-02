/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** InGameMenuOverlay.cpp
*/

#include "Client/Scenes/InGameMenuOverlay.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>

#include "Client/Events/OverlayEvents.hpp"
#include "Client/Scenes/SceneRoute.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UICancelEvent;
using engine::ecs::components::UIColor;

constexpr const char *RESUME = "Resume";
constexpr const char *SETTINGS = "Settings";
constexpr const char *CONTROLS = "Controls";
constexpr const char *DISCONNECT = "Disconnect";
constexpr std::array<const char *, 4> LABELS{RESUME, SETTINGS, CONTROLS,
                                             DISCONNECT};

constexpr UIColor BACKDROP_COLOR{0, 0, 0, 170};
constexpr unsigned int TITLE_TEXT_SIZE = 48;
constexpr Vec2 TITLE_OFFSET{0.0f, -180.0f};
constexpr Vec2 TITLE_SIZE{600.0f, 60.0f};
constexpr Vec2 BUTTON_SIZE{320.0f, 52.0f};
constexpr float FIRST_BUTTON_Y = -90.0f;
constexpr float BUTTON_SPACING = 66.0f;

} // namespace

InGameMenuOverlay::InGameMenuOverlay(engine::ecs::UISceneContext context)
    : UIOverlay(std::move(context))
{}

Entity InGameMenuOverlay::button(const std::string &label) const
{
    for (std::size_t i = 0; i < LABELS.size(); ++i) {
        if (label == LABELS.at(i))
            return _buttons.at(i);
    }
    throw std::out_of_range("no menu button: " + label);
}

void InGameMenuOverlay::activate()
{
    float y = FIRST_BUTTON_Y;

    addBackdrop(BACKDROP_COLOR);
    addLabel("Menu", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE);
    _buttons.clear();
    for (const char *label : LABELS) {
        _buttons.push_back(addButton(label, label, {0.0f, y}, BUTTON_SIZE));
        y += BUTTON_SPACING;
    }
    widget<UIButton>(_buttons.front()).isFocused = true;
    listen<UIButtonClickedEvent>(
        [this](const UIButtonClickedEvent &event) { onButton(event); });
    listen<UICancelEvent>([this](const UICancelEvent &) {
        bus().publish(events::ResumeRequested{});
    });
}

void InGameMenuOverlay::onButton(const UIButtonClickedEvent &event)
{
    if (event.action == RESUME) {
        bus().publish(events::ResumeRequested{});
    } else if (event.action == SETTINGS) {
        bus().publish(OverlayRequested{SceneRoute::SETTINGS});
    } else if (event.action == CONTROLS) {
        bus().publish(OverlayRequested{SceneRoute::CONTROLS});
    } else if (event.action == DISCONNECT) {
        bus().publish(events::DisconnectRequested{});
        bus().publish(NavigationRequested{SceneRoute::MAIN_MENU});
    }
}

} // namespace client::scenes
