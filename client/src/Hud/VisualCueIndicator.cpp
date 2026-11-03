/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** VisualCueIndicator.cpp
*/

#include "Client/Hud/VisualCueIndicator.hpp"

#include <cstddef>
#include <utility>

#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"

namespace client::hud {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::UIColorRole;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextAlign;
using engine::ecs::components::UITransform;
using events::CueDirection;
using events::CueIcon;

constexpr UIColor HIDDEN{0, 0, 0, 0};
constexpr UIColor PANEL_COLOR{0, 0, 0, 200};
constexpr UIColor TEXT_COLOR{255, 215, 0, 255};
constexpr Vec2 CUE_SIZE{170.0f, 34.0f};
constexpr unsigned int CUE_TEXT_SIZE = 20;
constexpr int CUE_Z_INDEX = 20;
constexpr float EDGE_GAP = 16.0f;
constexpr float BOTTOM_GAP = 60.0f;

struct Placement {
    UIAnchor anchor;
    Vec2 offset;
};

Placement placementOf(CueDirection direction) noexcept
{
    switch (direction) {
        case CueDirection::RIGHT:
            return {UIAnchor::CENTER_RIGHT, {-EDGE_GAP, 0.0f}};
        case CueDirection::TOP:
            return {UIAnchor::TOP_CENTER, {0.0f, EDGE_GAP}};
        case CueDirection::BOTTOM:
            return {UIAnchor::BOTTOM_CENTER, {0.0f, -BOTTOM_GAP}};
        default:
            return {UIAnchor::CENTER_LEFT, {EDGE_GAP, 0.0f}};
    }
}

const char *iconName(CueIcon icon) noexcept
{
    switch (icon) {
        case CueIcon::EXPLOSION:
            return "Explosion";
        case CueIcon::ENEMY:
            return "Enemy";
        case CueIcon::SHOT:
            return "Shot";
        default:
            return "Alert";
    }
}

} // namespace

VisualCueIndicator::VisualCueIndicator(engine::ecs::Registry &registry,
                                       engine::core::EventBus &bus,
                                       const std::string &fontId,
                                       float durationSeconds)
    : _registry(registry),
      _bus(bus),
      _listener(_bus.subscribe<events::VisualCueRequested>(
          [this](const events::VisualCueRequested &cue) { show(cue); })),
      _duration(durationSeconds),
      _sides{{{registry.spawnEntity()},
              {registry.spawnEntity()},
              {registry.spawnEntity()},
              {registry.spawnEntity()}}}
{
    for (const CueDirection direction :
         {CueDirection::LEFT, CueDirection::RIGHT, CueDirection::TOP,
          CueDirection::BOTTOM})
        build(direction, fontId);
}

VisualCueIndicator::~VisualCueIndicator()
{
    _bus.unsubscribe(_listener);
    for (const Side &side : _sides)
        _registry.killEntity(side.entity);
}

void VisualCueIndicator::update(float dt)
{
    for (Side &side : _sides) {
        if (side.timeLeft <= 0.0f)
            continue;
        side.timeLeft -= dt;
        if (side.timeLeft <= 0.0f)
            setVisible(side, "");
    }
}

Entity VisualCueIndicator::label(CueDirection direction) const
{
    return _sides.at(static_cast<std::size_t>(direction)).entity;
}

std::string VisualCueIndicator::cueText(CueIcon icon, CueDirection direction)
{
    const std::string name = iconName(icon);

    switch (direction) {
        case CueDirection::RIGHT:
            return name + " >>";
        case CueDirection::TOP:
            return "^ " + name;
        case CueDirection::BOTTOM:
            return "v " + name;
        default:
            return "<< " + name;
    }
}

void VisualCueIndicator::build(CueDirection direction,
                               const std::string &fontId)
{
    const Entity entity = label(direction);
    const Placement place = placementOf(direction);
    UITransform transform;
    UIPanel panel;

    panel.color = HIDDEN;
    transform.anchor = place.anchor;
    transform.offset = place.offset;
    transform.size = CUE_SIZE;
    transform.zIndex = CUE_Z_INDEX;
    _registry.addComponent(entity, transform);
    _registry.addComponent(entity, panel);
    _registry.addComponent(entity,
                           UIText{"", fontId, CUE_TEXT_SIZE, TEXT_COLOR,
                                  UITextAlign::CENTER, UIColorRole::WARNING});
}

void VisualCueIndicator::show(const events::VisualCueRequested &cue)
{
    Side &side = _sides.at(static_cast<std::size_t>(cue.direction));

    side.timeLeft = _duration;
    setVisible(side, cueText(cue.icon, cue.direction));
}

void VisualCueIndicator::setVisible(const Side &side, const std::string &text)
{
    _registry.getComponents<UIText>().get(side.entity)->text = text;
    _registry.getComponents<UIPanel>().get(side.entity)->color =
        text.empty() ? HIDDEN : PANEL_COLOR;
}

} // namespace client::hud
