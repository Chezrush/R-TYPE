/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UILayoutSystem.hpp
*/

#ifndef UILAYOUTSYSTEM_HPP_
#define UILAYOUTSYSTEM_HPP_

#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Phase.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"

namespace engine::ecs::systems {

/**
 * @brief System responsible for resolving UI bounding boxes.
 *
 * Computes absolute viewport coordinates (actualBounds) based on UIAnchor,
 * offset, size, and the current screen/viewport resolution. UITransform must
 * be registered in the registry (see registerUIComponents()).
 */
class UILayoutSystem {
public:
    explicit UILayoutSystem(float windowWidth = 1280.0f,
                            float windowHeight = 720.0f) noexcept;

    /** @brief Sets viewport resolution for layout recalculation. */
    void setResolution(float windowWidth, float windowHeight) noexcept;

    /** @brief Current viewport width. */
    [[nodiscard]] float windowWidth() const noexcept;

    /** @brief Current viewport height. */
    [[nodiscard]] float windowHeight() const noexcept;

    /**
     * @brief Computes absolute viewport bounds for a given UITransform.
     */
    [[nodiscard]] static components::Rect
    computeBounds(const components::UITransform &transform, float windowWidth,
                  float windowHeight) noexcept;

    /** @brief Resolves the bounds of every UITransform of the registry. */
    void update(Registry &registry) const;

    /**
     * @brief Registers the layout as a scheduler system (writes UITransform).
     *
     * The system reads the resolution of this object at each run, so this
     * object must outlive the scheduler entry. POST_UPDATE by default: the
     * bounds are resolved after gameplay changed the UI, before rendering.
     */
    Scheduler::SystemId addTo(Scheduler &scheduler,
                              Phase phase = Phase::POST_UPDATE) const;

private:
    float _windowWidth;
    float _windowHeight;
};

} // namespace engine::ecs::systems

namespace engine::ecs {

using systems::UILayoutSystem;

} // namespace engine::ecs

#endif /* !UILAYOUTSYSTEM_HPP_ */
