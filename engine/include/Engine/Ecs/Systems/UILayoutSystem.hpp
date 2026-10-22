/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UILayoutSystem.hpp
*/

#ifndef UILAYOUTSYSTEM_HPP_
#define UILAYOUTSYSTEM_HPP_

#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/SparseArray.hpp"

namespace engine::ecs::systems {

/**
 * @brief System responsible for resolving UI bounding boxes.
 *
 * Computes absolute viewport coordinates (actualBounds) based on UIAnchor,
 * offset, size, and the current screen/viewport resolution.
 */
class UILayoutSystem {
public:
    explicit UILayoutSystem(
        float windowWidth = 1280.0f,
        float windowHeight = 720.0f
    ) noexcept;

    /** @brief Sets viewport resolution for layout recalculation. */
    void setResolution(float windowWidth, float windowHeight) noexcept;

    /** @brief Current viewport width. */
    [[nodiscard]] float windowWidth() const noexcept;

    /** @brief Current viewport height. */
    [[nodiscard]] float windowHeight() const noexcept;

    /**
     * @brief Computes absolute viewport bounds for a given UITransform.
     */
    [[nodiscard]] static components::Rect computeBounds(
        const components::UITransform &transform,
        float windowWidth,
        float windowHeight
    ) noexcept;

    /**
     * @brief Updates all UITransform components in the sparse array.
     */
    void update(SparseArray<components::UITransform> &transforms) const;

    /** @brief Callable operator for ECS registry integration. */
    void operator()(SparseArray<components::UITransform> &transforms) const;

private:
    float _windowWidth;
    float _windowHeight;
};

} // namespace engine::ecs::systems

namespace engine::ecs {

using systems::UILayoutSystem;

} // namespace engine::ecs

#endif /* !UILAYOUTSYSTEM_HPP_ */

