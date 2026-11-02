/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIScene.cpp
*/

#include "Engine/Ecs/UIScene.hpp"

#include <utility>

namespace engine::ecs {

namespace {

using components::UIAnchor;
using components::UIButton;
using components::UIText;
using components::UITextAlign;
using components::UITransform;
using core::Vec2;

} // namespace

UIScene::UIScene(UISceneContext context)
    : _context(std::move(context))
{}

UIScene::~UIScene()
{
    deactivate();
}

void UIScene::onEnter()
{
    _active = true;
    activate();
}

void UIScene::onExit()
{
    deactivate();
}

void UIScene::onPause()
{
    deactivate();
}

void UIScene::onResume()
{
    _active = true;
    activate();
}

void UIScene::update(float /*dt*/) {}

void UIScene::render() {}

void UIScene::handleInput()
{
    _context.interaction.process(_context.registry);
}

bool UIScene::isOverlay() const noexcept
{
    return false;
}

void UIScene::setViewport(Vec2 viewport) noexcept
{
    _context.viewport = viewport;
}

Entity UIScene::spawn(UIAnchor anchor, Vec2 offset, Vec2 size, int zIndex)
{
    const Entity entity = _context.registry.spawnEntity();
    UITransform transform;

    transform.anchor = anchor;
    transform.offset = offset;
    transform.size = size;
    transform.zIndex = _context.baseZIndex + zIndex;
    _context.registry.addComponent(entity, transform);
    _entities.push_back(entity);
    return entity;
}

Entity UIScene::addLabel(const std::string &text, Vec2 offset, Vec2 size,
                         unsigned int characterSize)
{
    const Entity entity = spawn(UIAnchor::CENTER, offset, size);
    UIText label;

    label.text = text;
    label.fontId = _context.fontId;
    label.characterSize = characterSize;
    label.align = UITextAlign::CENTER;
    _context.registry.addComponent(entity, std::move(label));
    return entity;
}

Entity UIScene::addButton(const std::string &text, const std::string &action,
                          Vec2 offset, Vec2 size)
{
    const Entity entity = addLabel(text, offset, size, _context.buttonTextSize);
    UIButton button = _context.buttonStyle;

    button.action = action;
    _context.registry.addComponent(entity, std::move(button));
    return entity;
}

engine::core::EventBus &UIScene::bus() noexcept
{
    return _context.bus;
}

engine::ecs::Registry &UIScene::registry() noexcept
{
    return _context.registry;
}

const std::string &UIScene::fontId() const noexcept
{
    return _context.fontId;
}

bool UIScene::isActive() const noexcept
{
    return _active;
}

Vec2 UIScene::viewport() const noexcept
{
    return _context.viewport;
}

void UIScene::deactivate()
{
    _active = false;
    for (const auto listener : _listeners)
        _context.bus.unsubscribe(listener);
    _listeners.clear();
    for (const Entity entity : _entities)
        _context.registry.killEntity(entity);
    _entities.clear();
}

} // namespace engine::ecs
