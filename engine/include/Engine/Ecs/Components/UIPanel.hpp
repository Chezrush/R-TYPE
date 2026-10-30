/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIPanel.hpp
*/

#ifndef UIPANEL_HPP_
#define UIPANEL_HPP_

#include <cstdint>
#include <string>

#include "Engine/Ecs/Components/UIColor.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"

namespace engine::ecs::components {

/**
 * @brief How a textured panel is stretched over its bounds.
 */
enum class UIPanelMode : std::uint8_t {
    STRETCH,
    NINE_SLICE
};

/**
 * @brief Border sizes, in texture pixels, kept unscaled by 9-slicing.
 */
struct UIBorders {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    friend bool operator==(const UIBorders &, const UIBorders &) = default;
};

/**
 * @brief Background quad filling the entity's UITransform bounds.
 *
 * Without textureId the panel is a plain rectangle of the given color.
 * With one, color tints the texture. An empty textureRect (zero size) uses
 * the whole texture. In NINE_SLICE mode the corners keep their size, the
 * edges stretch along one axis and the center along both.
 */
struct UIPanel {
    UIColor color{0, 0, 0, 160};
    std::string textureId;
    Rect textureRect{0.0f, 0.0f, 0.0f, 0.0f};
    UIPanelMode mode = UIPanelMode::STRETCH;
    UIBorders borders;

    friend bool operator==(const UIPanel &, const UIPanel &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIBorders;
using components::UIPanel;
using components::UIPanelMode;

} // namespace engine::ecs

#endif /* !UIPANEL_HPP_ */
