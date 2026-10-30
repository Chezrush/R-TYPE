/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIDropdown.hpp
*/

#ifndef UIDROPDOWN_HPP_
#define UIDROPDOWN_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include "Engine/Ecs/Components/UIWidgetStyle.hpp"

namespace engine::ecs::components {

/**
 * @brief Closed box showing the selected option, opening a list below it.
 *
 * Each option row has the height of the bounds and is drawn below them, on
 * top of every other widget. highlighted is the option under the pointer or
 * the keyboard while the list is open.
 */
struct UIDropdown {
    std::vector<std::string> options;
    std::size_t selected = 0;
    std::size_t highlighted = 0;
    bool open = false;
    bool isFocused = false;
    bool enabled = true;
    std::string action;
    UIWidgetStyle style;

    friend bool operator==(const UIDropdown &, const UIDropdown &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIDropdown;

} // namespace engine::ecs

#endif /* !UIDROPDOWN_HPP_ */
