/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameOverOverlay.cpp
*/

#include "Client/Scenes/GameOverOverlay.hpp"

#include <utility>

#include "Client/Events/OverlayEvents.hpp"
#include "Client/Hud/HudView.hpp"
#include "Client/Scenes/SceneRoute.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;

constexpr engine::ecs::components::UIColor BACKDROP_COLOR{0, 0, 0, 200};
constexpr unsigned int TITLE_TEXT_SIZE = 72;
constexpr unsigned int SCORE_TEXT_SIZE = 32;
constexpr Vec2 TITLE_OFFSET{0.0f, -120.0f};
constexpr Vec2 TITLE_SIZE{800.0f, 90.0f};
constexpr Vec2 SCORE_OFFSET{0.0f, -30.0f};
constexpr Vec2 SCORE_SIZE{600.0f, 44.0f};
constexpr Vec2 BUTTON_OFFSET{0.0f, 60.0f};
constexpr Vec2 BUTTON_SIZE{320.0f, 52.0f};

} // namespace

GameOverOverlay::GameOverOverlay(engine::ecs::UISceneContext context,
                                 std::uint32_t score)
    : UIOverlay(std::move(context)),
      _score(score)
{}

std::optional<engine::ecs::Entity> GameOverOverlay::scoreLabel() const noexcept
{
    return _scoreLabel;
}

std::optional<engine::ecs::Entity> GameOverOverlay::backButton() const noexcept
{
    return _back;
}

void GameOverOverlay::activate()
{
    addBackdrop(BACKDROP_COLOR);
    addLabel("GAME OVER", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE);
    _scoreLabel = addLabel("Final score " + hud::HudView::formatScore(_score),
                           SCORE_OFFSET, SCORE_SIZE, SCORE_TEXT_SIZE);
    const engine::ecs::Entity back =
        addButton("Back to menu", "game-over.back", BUTTON_OFFSET, BUTTON_SIZE);

    _back = back;
    widget<UIButton>(back).isFocused = true;
    listen<UIButtonClickedEvent>([this](const UIButtonClickedEvent &) {
        bus().publish(events::DisconnectRequested{});
        bus().publish(NavigationRequested{SceneRoute::MAIN_MENU});
    });
}

} // namespace client::scenes
