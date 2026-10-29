/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIText.hpp
*/

#ifndef UITEXT_HPP_
#define UITEXT_HPP_

#include <cstdint>
#include <string>

#include "Engine/Ecs/Components/UIColor.hpp"

namespace engine::ecs::components {

/**
 * @brief Horizontal placement of a text inside its UITransform bounds.
 */
enum class UITextAlign : std::uint8_t {
    LEFT,
    CENTER,
    RIGHT
};

/**
 * @brief Text label drawn inside the entity's UITransform bounds.
 *
 * The text is vertically centered in the bounds. fontId is the path of the
 * font, relative to the ResourceManager asset root. A missing font only hides
 * the label, it never stops the frame.
 */
struct UIText {
    std::string text;
    std::string fontId;
    unsigned int characterSize = 24;
    UIColor color{255, 255, 255, 255};
    UITextAlign align = UITextAlign::LEFT;

    friend bool operator==(const UIText &, const UIText &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIText;
using components::UITextAlign;

} // namespace engine::ecs

#endif /* !UITEXT_HPP_ */
