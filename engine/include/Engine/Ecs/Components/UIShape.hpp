/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIShape.hpp
*/

#ifndef UISHAPE_HPP_
#define UISHAPE_HPP_

#include <cstdint>

#include "Engine/Ecs/Components/UIColor.hpp"

namespace engine::ecs::components {

/** @brief Simple figures, so that nothing is told apart by color alone. */
enum class UIShapeKind : std::uint8_t {
    CIRCLE,
    TRIANGLE,
    SQUARE,
    CROSS
};

/** @brief Figure filling the entity's UITransform bounds. */
struct UIShape {
    UIShapeKind kind = UIShapeKind::SQUARE;
    UIColor color{255, 255, 255, 255};
    UIColorRole colorRole = UIColorRole::NONE;

    friend bool operator==(const UIShape &, const UIShape &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIShape;
using components::UIShapeKind;

} // namespace engine::ecs

#endif /* !UISHAPE_HPP_ */
