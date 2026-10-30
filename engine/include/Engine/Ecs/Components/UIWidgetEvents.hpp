/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIWidgetEvents.hpp
*/

#ifndef UIWIDGETEVENTS_HPP_
#define UIWIDGETEVENTS_HPP_

#include <cstddef>
#include <string>

namespace engine::ecs::components {

/** @brief The text of a UITextInput changed (typing, erasing). */
struct UITextChangedEvent {
    std::size_t entityIndex = 0;
    std::string action;
    std::string text;

    friend bool operator==(const UITextChangedEvent &,
                           const UITextChangedEvent &) = default;
};

/**
 * @brief The value of a UISlider or a UIToggle changed.
 *
 * A toggle sends 1 when checked, 0 when unchecked.
 */
struct UIValueChangedEvent {
    std::size_t entityIndex = 0;
    std::string action;
    float value = 0.0f;

    friend bool operator==(const UIValueChangedEvent &,
                           const UIValueChangedEvent &) = default;
};

/** @brief The selected option of a UIDropdown or row of a UIScrollList. */
struct UISelectionChangedEvent {
    std::size_t entityIndex = 0;
    std::string action;
    std::size_t index = 0;
    std::string label;

    friend bool operator==(const UISelectionChangedEvent &,
                           const UISelectionChangedEvent &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UISelectionChangedEvent;
using components::UITextChangedEvent;
using components::UIValueChangedEvent;

} // namespace engine::ecs

#endif /* !UIWIDGETEVENTS_HPP_ */
