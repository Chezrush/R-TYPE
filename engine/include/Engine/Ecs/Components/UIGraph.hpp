/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIGraph.hpp
*/

#ifndef UIGRAPH_HPP_
#define UIGRAPH_HPP_

#include <cstddef>
#include <vector>

#include "Engine/Ecs/Components/UIColor.hpp"

namespace engine::ecs::components {

/**
 * @brief Rolling bar graph (frame time, latency...) filling its bounds.
 *
 * samples are drawn oldest first, newest on the right, one bar per slot of
 * capacity. maxValue is the top of the graph; 0 or less scales it to the
 * largest sample. A bar never goes above the bounds.
 */
struct UIGraph {
    std::vector<float> samples;
    std::size_t capacity = 120;
    float maxValue = 0.0f;
    UIColor barColor{110, 190, 255, 255};
    UIColor backgroundColor{0, 0, 0, 140};
    UIColorRole barRole = UIColorRole::NONE;
    UIColorRole backgroundRole = UIColorRole::NONE;

    friend bool operator==(const UIGraph &, const UIGraph &) = default;
};

} // namespace engine::ecs::components

namespace engine::ecs {

using components::UIGraph;

} // namespace engine::ecs

#endif /* !UIGRAPH_HPP_ */
