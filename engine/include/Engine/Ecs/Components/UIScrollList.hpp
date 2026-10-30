/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIScrollList.hpp
*/

#ifndef UISCROLLLIST_HPP_
#define UISCROLLLIST_HPP_

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Engine/Ecs/Components/UIWidgetStyle.hpp"

namespace engine::ecs::components {

/**
 * @brief Vertical list of text rows, scrolled inside its bounds.
 *
 * The bounds are split into visibleRows rows (at least one). scrollOffset is
 * the first visible row. The mouse wheel scrolls, a click or Up / Down select.
 */
struct UIScrollList {
    std::vector<std::string> rows;
    std::size_t scrollOffset = 0;
    std::optional<std::size_t> selected;
    std::size_t visibleRows = 5;
    bool isFocused = false;
    bool enabled = true;
    std::string action;
    UIWidgetStyle style;

    friend bool operator==(const UIScrollList &,
                           const UIScrollList &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIScrollList;

} // namespace engine::ecs

#endif /* !UISCROLLLIST_HPP_ */
