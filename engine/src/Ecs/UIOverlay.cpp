/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIOverlay.cpp
*/

#include "Engine/Ecs/UIOverlay.hpp"

#include <utility>

#include "Engine/Ecs/Components/UIPanel.hpp"

namespace engine::ecs {

namespace {

constexpr int BACKDROP_Z_INDEX = -1;

UISceneContext onTop(UISceneContext context)
{
    context.baseZIndex += UI_OVERLAY_Z_INDEX;
    return context;
}

} // namespace

UIOverlay::UIOverlay(UISceneContext context, bool updatesBelow)
    : UIScene(onTop(std::move(context))),
      _updatesBelow(updatesBelow)
{}

bool UIOverlay::isOverlay() const noexcept
{
    return true;
}

bool UIOverlay::updatesSceneBelow() const noexcept
{
    return _updatesBelow;
}

Entity UIOverlay::addBackdrop(components::UIColor color)
{
    const Entity entity = spawn(components::UIAnchor::TOP_LEFT, {0.0f, 0.0f},
                                {1.0f, 1.0f}, BACKDROP_Z_INDEX);
    components::UIPanel panel;

    panel.color = color;
    widget<components::UITransform>(entity).normalizedSize = true;
    registry().addComponent(entity, std::move(panel));
    return entity;
}

} // namespace engine::ecs
