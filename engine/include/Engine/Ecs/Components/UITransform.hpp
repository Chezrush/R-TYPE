/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UITransform.hpp
*/

#ifndef UITRANSFORM_HPP_
#define UITRANSFORM_HPP_

#include <compare>

#include "Engine/Core/Vec2.hpp"

namespace engine::ecs::components {

/**
 * @brief Standard 9-anchor attachment points for UI layout.
 */
enum class UIAnchor {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

/**
 * @brief 2D rectangle representing resolved viewport boundaries.
 */
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    [[nodiscard]] bool operator==(const Rect &) const noexcept = default;
};

/**
 * @brief Component representing viewport-space transform of a UI entity.
 */
struct UITransform {
    UIAnchor anchor = UIAnchor::TOP_LEFT;
    core::Vec2 offset{0.0f, 0.0f};
    core::Vec2 size{0.0f, 0.0f};
    int zIndex = 0;
    bool normalizedSize = false;
    Rect actualBounds{0.0f, 0.0f, 0.0f, 0.0f};

    [[nodiscard]] auto operator<=>(const UITransform &other) const noexcept
    {
        return zIndex <=> other.zIndex;
    }

    [[nodiscard]] bool operator==(const UITransform &) const noexcept = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::Rect;
using components::UIAnchor;
using components::UITransform;

} // namespace engine::ecs

#endif /* !UITRANSFORM_HPP_ */
