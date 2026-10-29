/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIColor.hpp
*/

#ifndef UICOLOR_HPP_
#define UICOLOR_HPP_

#include <cstdint>

namespace engine::ecs::components {

/**
 * @brief RGBA color shared by every UI component.
 */
struct UIColor {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    friend bool operator==(const UIColor &, const UIColor &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIColor;

} // namespace engine::ecs

#endif /* !UICOLOR_HPP_ */
