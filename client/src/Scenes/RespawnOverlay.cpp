/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RespawnOverlay.cpp
*/

#include "Client/Scenes/RespawnOverlay.hpp"

#include <string>
#include <utility>

#include "Engine/Ecs/Components/UIText.hpp"

namespace client::scenes {

namespace {

constexpr engine::ecs::components::UIColor BACKDROP_COLOR{120, 0, 0, 60};
constexpr unsigned int TEXT_SIZE = 40;
constexpr engine::core::Vec2 LABEL_OFFSET{0.0f, -60.0f};
constexpr engine::core::Vec2 LABEL_SIZE{800.0f, 60.0f};

} // namespace

RespawnOverlay::RespawnOverlay(engine::ecs::UISceneContext context,
                               const GameOverlays &overlays)
    : UIOverlay(std::move(context)),
      _overlays(overlays)
{}

void RespawnOverlay::update(float /*dt*/)
{
    syncLabel();
}

std::optional<engine::ecs::Entity> RespawnOverlay::label() const noexcept
{
    return _label;
}

void RespawnOverlay::activate()
{
    addBackdrop(BACKDROP_COLOR);
    _label = addLabel("", LABEL_OFFSET, LABEL_SIZE, TEXT_SIZE);
    syncLabel();
}

void RespawnOverlay::syncLabel()
{
    if (_label.has_value())
        widget<engine::ecs::components::UIText>(*_label).text =
            "Respawning in " + std::to_string(_overlays.respawnSecondsLeft());
}

} // namespace client::scenes
