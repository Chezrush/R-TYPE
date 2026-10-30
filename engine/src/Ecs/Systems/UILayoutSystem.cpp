/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UILayoutSystem.cpp
*/

#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

namespace engine::ecs::systems {

namespace {

components::Vec2 computeSize(const components::UITransform &transform,
                             float windowWidth, float windowHeight) noexcept
{
    if (transform.normalizedSize) {
        return {transform.size.x * windowWidth,
                transform.size.y * windowHeight};
    }
    return transform.size;
}

float computeX(components::UIAnchor anchor, float offsetX, float width,
               float windowWidth) noexcept
{
    using components::UIAnchor;
    switch (anchor) {
        case UIAnchor::TOP_CENTER:
        case UIAnchor::CENTER:
        case UIAnchor::BOTTOM_CENTER:
            return (windowWidth - width) * 0.5f + offsetX;
        case UIAnchor::TOP_RIGHT:
        case UIAnchor::CENTER_RIGHT:
        case UIAnchor::BOTTOM_RIGHT:
            return windowWidth - width + offsetX;
        default:
            return offsetX;
    }
}

float computeY(components::UIAnchor anchor, float offsetY, float height,
               float windowHeight) noexcept
{
    using components::UIAnchor;
    switch (anchor) {
        case UIAnchor::CENTER_LEFT:
        case UIAnchor::CENTER:
        case UIAnchor::CENTER_RIGHT:
            return (windowHeight - height) * 0.5f + offsetY;
        case UIAnchor::BOTTOM_LEFT:
        case UIAnchor::BOTTOM_CENTER:
        case UIAnchor::BOTTOM_RIGHT:
            return windowHeight - height + offsetY;
        default:
            return offsetY;
    }
}

} // namespace

UILayoutSystem::UILayoutSystem(float windowWidth, float windowHeight) noexcept
    : _windowWidth(windowWidth),
      _windowHeight(windowHeight)
{}

void UILayoutSystem::setResolution(float windowWidth,
                                   float windowHeight) noexcept
{
    _windowWidth = windowWidth;
    _windowHeight = windowHeight;
}

float UILayoutSystem::windowWidth() const noexcept
{
    return _windowWidth;
}

float UILayoutSystem::windowHeight() const noexcept
{
    return _windowHeight;
}

components::Rect
UILayoutSystem::computeBounds(const components::UITransform &transform,
                              float windowWidth, float windowHeight) noexcept
{
    const components::Vec2 size =
        computeSize(transform, windowWidth, windowHeight);
    const float x =
        computeX(transform.anchor, transform.offset.x, size.x, windowWidth);
    const float y =
        computeY(transform.anchor, transform.offset.y, size.y, windowHeight);

    return {x, y, size.x, size.y};
}

void UILayoutSystem::update(
    SparseArray<components::UITransform> &transforms) const
{
    for (auto &slot : transforms) {
        if (slot.has_value()) {
            slot->actualBounds =
                computeBounds(*slot, _windowWidth, _windowHeight);
        }
    }
}

void UILayoutSystem::operator()(
    SparseArray<components::UITransform> &transforms) const
{
    update(transforms);
}

} // namespace engine::ecs::systems
