/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UITransform.hpp
*/

#ifndef UITRANSFORM_HPP_
#define UITRANSFORM_HPP_

#include <compare>

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
 * @brief 2D float vector for UI positioning and sizing.
 */
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    [[nodiscard]] bool operator==(const Vec2 &) const noexcept = default;
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
    Vec2 offset{0.0f, 0.0f};
    Vec2 size{0.0f, 0.0f};
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
using components::Vec2;

} // namespace engine::ecs

#endif /* !UITRANSFORM_HPP_ */

