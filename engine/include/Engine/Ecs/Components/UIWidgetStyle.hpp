/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIWidgetStyle.hpp
*/

#ifndef UIWIDGETSTYLE_HPP_
#define UIWIDGETSTYLE_HPP_

#include <string>

#include "Engine/Ecs/Components/UIColor.hpp"

namespace engine::ecs::components {

/**
 * @brief Look shared by the interactive widgets.
 *
 * background fills the widget, foreground is used for text and handles,
 * accent for the filled / selected / checked parts, focusOutline for the
 * frame drawn around the focused widget. fontId is resolved by the
 * ResourceManager; a missing font only hides the text.
 */
struct UIWidgetStyle {
    UIColor background{30, 30, 40, 230};
    UIColor foreground{235, 235, 245, 255};
    UIColor accent{70, 130, 255, 255};
    UIColor focusOutline{255, 215, 0, 255};
    std::string fontId;
    unsigned int characterSize = 20;

    friend bool operator==(const UIWidgetStyle &,
                           const UIWidgetStyle &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIWidgetStyle;

} // namespace engine::ecs

#endif /* !UIWIDGETSTYLE_HPP_ */
