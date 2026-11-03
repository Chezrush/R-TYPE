/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MainMenuScene.cpp
*/

#include "Client/Scenes/MainMenuScene.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>

#include "Client/Scenes/SceneRoute.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UITransform;

struct MenuEntry {
    const char *label;
    std::optional<SceneRoute> route;
    bool overlay;
};

constexpr std::array<MenuEntry, 5> ENTRIES{{
    {"Play", SceneRoute::CONNECT, false},
    {"Settings", SceneRoute::SETTINGS, true},
    {"Controls", SceneRoute::CONTROLS, true},
    {"How to play", SceneRoute::HOW_TO_PLAY, true},
    {"Quit", std::nullopt, false},
}};

constexpr std::size_t STAR_COUNT = 120;
constexpr std::uint32_t STAR_SEED = 1987;
constexpr int STAR_Z_INDEX = -10;
constexpr unsigned int LOGO_TEXT_SIZE = 84;
constexpr Vec2 LOGO_OFFSET{0.0f, -170.0f};
constexpr Vec2 LOGO_SIZE{800.0f, 100.0f};
constexpr Vec2 BUTTON_SIZE{320.0f, 52.0f};
constexpr float FIRST_BUTTON_Y = -80.0f;
constexpr float BUTTON_SPACING = 60.0f;

} // namespace

MainMenuScene::MainMenuScene(engine::ecs::UISceneContext context)
    : UIScene(std::move(context)),
      _starfield(STAR_COUNT, STAR_SEED)
{}

void MainMenuScene::update(float dt)
{
    if (!reducedMotion())
        _starfield.advance(dt);
    if (isActive())
        placeStars();
}

Entity MainMenuScene::button(const std::string &label) const
{
    for (std::size_t i = 0; i < ENTRIES.size(); ++i) {
        if (label == ENTRIES.at(i).label)
            return _buttons.at(i);
    }
    throw std::out_of_range("no menu button: " + label);
}

const Starfield &MainMenuScene::starfield() const noexcept
{
    return _starfield;
}

void MainMenuScene::activate()
{
    buildStars();
    addLabel("R-TYPE", LOGO_OFFSET, LOGO_SIZE, LOGO_TEXT_SIZE);
    buildButtons();
    listen<UIButtonClickedEvent>(
        [this](const UIButtonClickedEvent &event) { onButton(event); });
}

void MainMenuScene::buildStars()
{
    _stars.clear();
    for (const Star &star : _starfield.stars()) {
        const Entity entity =
            spawn(UIAnchor::TOP_LEFT, {}, {star.size, star.size}, STAR_Z_INDEX);
        UIPanel panel;

        widget<UITransform>(entity).followsUIScale = false;

        panel.color =
            UIColor{star.brightness, star.brightness, star.brightness, 255};
        registry().addComponent(entity, std::move(panel));
        _stars.push_back(entity);
    }
    placeStars();
}

void MainMenuScene::buildButtons()
{
    float y = FIRST_BUTTON_Y;

    _buttons.clear();
    for (const MenuEntry &entry : ENTRIES) {
        _buttons.push_back(
            addButton(entry.label, entry.label, {0.0f, y}, BUTTON_SIZE));
        y += BUTTON_SPACING;
    }
    widget<UIButton>(_buttons.front()).isFocused = true;
}

void MainMenuScene::placeStars()
{
    const std::vector<Star> &stars = _starfield.stars();

    for (std::size_t i = 0; i < _stars.size(); ++i)
        widget<UITransform>(_stars.at(i)).offset = {
            stars.at(i).position.x * viewport().x,
            stars.at(i).position.y * viewport().y};
}

void MainMenuScene::onButton(const UIButtonClickedEvent &event)
{
    for (const MenuEntry &entry : ENTRIES) {
        if (event.action != entry.label)
            continue;
        if (!entry.route.has_value())
            bus().publish(engine::core::QuitRequested{});
        else if (entry.overlay)
            bus().publish(OverlayRequested{*entry.route});
        else
            bus().publish(NavigationRequested{*entry.route});
    }
}

} // namespace client::scenes
